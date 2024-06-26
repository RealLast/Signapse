/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// independent from idl_parser, since this code is not needed for most clients
#include <cassert>
#include <unordered_map>
#include <unordered_set>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

const std::string kGeneratedFileNamePostfix = "_generated";

struct JsLanguageParameters {
  IDLOptions::Language language;
  std::string file_extension;
};

struct ReexportDescription {
  std::string symbol;
  std::string source_namespace;
  std::string target_namespace;
};

const JsLanguageParameters &GetJsLangParams(IDLOptions::Language lang) {
  static JsLanguageParameters js_language_parameters[] = {
    {
        IDLOptions::kJs,
        ".js",
    },
    {
        IDLOptions::kTs,
        ".ts",
    },
  };

  if (lang == IDLOptions::kJs) {
    return js_language_parameters[0];
  } else {
    assert(lang == IDLOptions::kTs);
    return js_language_parameters[1];
  }
}

static std::string GeneratedFileName(const std::string &path,
                                     const std::string &file_name,
                                     const JsLanguageParameters &lang) {
  return path + file_name + kGeneratedFileNamePostfix + lang.file_extension;
}

namespace js {
// Iterate through all definitions we haven't generate code for (enums, structs,
// and tables) and output them to a single file.
class JsGenerator : public BaseGenerator {
 public:
  typedef std::unordered_set<std::string> imported_fileset;
  typedef std::unordered_multimap<std::string, ReexportDescription>
      reexport_map;

  JsGenerator(const Parser &parser, const std::string &path,
              const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "."),
        lang_(GetJsLangParams(parser_.opts.lang)){};
  // Iterate through all definitions we haven't generate code for (enums,
  // structs, and tables) and output them to a single file.
  bool generate() {
    imported_fileset imported_files;
    reexport_map reexports;

    std::string enum_code, struct_code, import_code, exports_code, code;
    generateEnums(&enum_code, &exports_code, reexports);
    generateStructs(&struct_code, &exports_code, imported_files);
    generateImportDependencies(&import_code, imported_files);
    generateReexports(&import_code, reexports, imported_files);

    code = code + "// " + FlatBuffersGeneratedWarning() + "\n\n";

    // Generate code for all the namespace declarations.
    GenNamespaces(&code, &exports_code);

    // Output the main declaration code from above.
    code += import_code;

    code += enum_code;
    code += struct_code;

    if (lang_.language == IDLOptions::kJs && !exports_code.empty() &&
        !parser_.opts.skip_js_exports) {
      code += "// Exports for Node.js and RequireJS\n";
      code += exports_code;
    }

    return SaveFile(GeneratedFileName(path_, file_name_, lang_).c_str(), code,
                    false);
  }

 private:
  JsLanguageParameters lang_;

  // Generate code for imports
  void generateImportDependencies(std::string *code_ptr,
                                  const imported_fileset &imported_files) {
    std::string &code = *code_ptr;
    for (auto it = imported_files.begin(); it != imported_files.end(); ++it) {
      const auto &file = *it;
      const auto basename =
          flatbuffers::StripPath(flatbuffers::StripExtension(file));
      if (basename != file_name_) {
        const auto file_name = basename + kGeneratedFileNamePostfix;
        code += GenPrefixedImport(file, file_name);
      }
    }
  }

  // Generate reexports, which might not have been explicitly imported using the
  // "export import" trick
  void generateReexports(std::string *code_ptr, const reexport_map &reexports,
                         imported_fileset imported_files) {
    if (!parser_.opts.reexport_ts_modules ||
        lang_.language != IDLOptions::kTs) {
      return;
    }

    std::string &code = *code_ptr;
    for (auto it = reexports.begin(); it != reexports.end(); ++it) {
      const auto &file = *it;
      const auto basename =
          flatbuffers::StripPath(flatbuffers::StripExtension(file.first));
      if (basename != file_name_) {
        const auto file_name = basename + kGeneratedFileNamePostfix;

        if (imported_files.find(file.first) == imported_files.end()) {
          code += GenPrefixedImport(file.first, file_name);
          imported_files.emplace(file.first);
        }

        code += "export namespace " + file.second.target_namespace + " { \n";
        code += "export import " + file.second.symbol + " = ";
        code += GenFileNamespacePrefix(file.first) + "." +
                file.second.source_namespace + "." + file.second.symbol +
                "; }\n";
      }
    }
  }

  // Generate code for all enums.
  void generateEnums(std::string *enum_code_ptr, std::string *exports_code_ptr,
                     reexport_map &reexports) {
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      GenEnum(enum_def, enum_code_ptr, exports_code_ptr, reexports);
    }
  }

  // Generate code for all structs.
  void generateStructs(std::string *decl_code_ptr,
                       std::string *exports_code_ptr,
                       imported_fileset &imported_files) {
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      GenStruct(parser_, struct_def, decl_code_ptr, exports_code_ptr,
                imported_files);
    }
  }
  void GenNamespaces(std::string *code_ptr, std::string *exports_ptr) {
    if (lang_.language == IDLOptions::kTs &&
        parser_.opts.skip_flatbuffers_import) {
      return;
    }

    std::set<std::string> namespaces;

    for (auto it = parser_.namespaces_.begin(); it != parser_.namespaces_.end();
         ++it) {
      std::string namespace_so_far;

      // Gather all parent namespaces for this namespace
      for (auto component = (*it)->components.begin();
           component != (*it)->components.end(); ++component) {
        if (!namespace_so_far.empty()) { namespace_so_far += '.'; }
        namespace_so_far += *component;
        namespaces.insert(namespace_so_far);
      }
    }

    // Make sure parent namespaces come before child namespaces
    std::vector<std::string> sorted_namespaces(namespaces.begin(),
                                               namespaces.end());
    std::sort(sorted_namespaces.begin(), sorted_namespaces.end());

    // Emit namespaces in a form that Closure Compiler can optimize
    std::string &code = *code_ptr;
    std::string &exports = *exports_ptr;
    for (auto it = sorted_namespaces.begin(); it != sorted_namespaces.end();
         it++) {
      if (lang_.language == IDLOptions::kTs) {
        if (it->find('.') == std::string::npos) {
          code += "import { flatbuffers } from \"./flatbuffers\"\n";
          break;
        }
      } else {
        code += "/**\n * @const\n * @namespace\n */\n";
        if (it->find('.') == std::string::npos) {
          code += "var ";
          if (parser_.opts.use_goog_js_export_format) {
            exports += "goog.exportSymbol('" + *it + "', " + *it + ");\n";
          } else {
            exports += "this." + *it + " = " + *it + ";\n";
          }
        }
        code += *it + " = " + *it + " || {};\n\n";
      }
    }
  }

  // Generate a documentation comment, if available.
  static void GenDocComment(const std::vector<std::string> &dc,
                            std::string *code_ptr,
                            const std::string &extra_lines,
                            const char *indent = nullptr) {
    if (dc.empty() && extra_lines.empty()) {
      // Don't output empty comment blocks with 0 lines of comment content.
      return;
    }

    std::string &code = *code_ptr;
    if (indent) code += indent;
    code += "/**\n";
    for (auto it = dc.begin(); it != dc.end(); ++it) {
      if (indent) code += indent;
      code += " *" + *it + "\n";
    }
    if (!extra_lines.empty()) {
      if (!dc.empty()) {
        if (indent) code += indent;
        code += " *\n";
      }
      if (indent) code += indent;
      std::string::size_type start = 0;
      for (;;) {
        auto end = extra_lines.find('\n', start);
        if (end != std::string::npos) {
          code += " * " + extra_lines.substr(start, end - start) + "\n";
          start = end + 1;
        } else {
          code += " * " + extra_lines.substr(start) + "\n";
          break;
        }
      }
    }
    if (indent) code += indent;
    code += " */\n";
  }

  static void GenDocComment(std::string *code_ptr,
                            const std::string &extra_lines) {
    GenDocComment(std::vector<std::string>(), code_ptr, extra_lines);
  }

  // Generate an enum declaration and an enum string lookup table.
  void GenEnum(EnumDef &enum_def, std::string *code_ptr,
               std::string *exports_ptr, reexport_map &reexports) {
    if (enum_def.generated) return;
    std::string &code = *code_ptr;
    std::string &exports = *exports_ptr;
    GenDocComment(enum_def.doc_comment, code_ptr, "@enum");
    std::string ns = GetNameSpace(enum_def);
    if (lang_.language == IDLOptions::kTs) {
      if (!ns.empty()) { code += "export namespace " + ns + "{\n"; }
      code += "export enum " + enum_def.name + "{\n";
    } else {
      if (enum_def.defined_namespace->components.empty()) {
        code += "var ";
        if (parser_.opts.use_goog_js_export_format) {
          exports += "goog.exportSymbol('" + enum_def.name + "', " +
                     enum_def.name + ");\n";
        } else {
          exports += "this." + enum_def.name + " = " + enum_def.name + ";\n";
        }
      }
      code += WrapInNameSpace(enum_def) + " = {\n";
    }
    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
         ++it) {
      auto &ev = **it;
      if (!ev.doc_comment.empty()) {
        if (it != enum_def.vals.vec.begin()) { code += '\n'; }
        GenDocComment(ev.doc_comment, code_ptr, "", "  ");
      }
      code += "  " + ev.name;
      code += lang_.language == IDLOptions::kTs ? "= " : ": ";
      code += NumToString(ev.value);
      code += (it + 1) != enum_def.vals.vec.end() ? ",\n" : "\n";

      if (ev.union_type.struct_def) {
        ReexportDescription desc = { ev.name,
                                     GetNameSpace(*ev.union_type.struct_def),
                                     GetNameSpace(enum_def) };
        reexports.insert(
            std::make_pair(ev.union_type.struct_def->file, std::move(desc)));
      }
    }

    if (lang_.language == IDLOptions::kTs && !ns.empty()) { code += "}"; }
    code += "};\n\n";
  }

  static std::string GenType(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_BOOL:
      case BASE_TYPE_CHAR: return "Int8";
      case BASE_TYPE_UTYPE:
      case BASE_TYPE_UCHAR: return "Uint8";
      case BASE_TYPE_SHORT: return "Int16";
      case BASE_TYPE_USHORT: return "Uint16";
      case BASE_TYPE_INT: return "Int32";
      case BASE_TYPE_UINT: return "Uint32";
      case BASE_TYPE_LONG: return "Int64";
      case BASE_TYPE_ULONG: return "Uint64";
      case BASE_TYPE_FLOAT: return "Float32";
      case BASE_TYPE_DOUBLE: return "Float64";
      case BASE_TYPE_STRING: return "String";
      case BASE_TYPE_VECTOR: return GenType(type.VectorType());
      case BASE_TYPE_STRUCT: return type.struct_def->name;
      default: return "Table";
    }
  }

  std::string GenGetter(const Type &type, const std::string &arguments) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return GenBBAccess() + ".__string" + arguments;
      case BASE_TYPE_STRUCT: return GenBBAccess() + ".__struct" + arguments;
      case BASE_TYPE_UNION: return GenBBAccess() + ".__union" + arguments;
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType(), arguments);
      default: {
        auto getter =
            GenBBAccess() + ".read" + MakeCamel(GenType(type)) + arguments;
        if (type.base_type == BASE_TYPE_BOOL) { getter = "!!" + getter; }
        if (type.enum_def) {
          getter = "/** @type {" + WrapInNameSpace(*type.enum_def) + "} */ (" +
                   getter + ")";
        }
        return getter;
      }
    }
  }

  std::string GenBBAccess() {
    return lang_.language == IDLOptions::kTs ? "this.bb!" : "this.bb";
  }

  std::string GenDefaultValue(const Value &value, const std::string &context) {
    if (value.type.enum_def) {
      if (auto val = value.type.enum_def->ReverseLookup(
              StringToInt(value.constant.c_str()), false)) {
        if (lang_.language == IDLOptions::kTs) {
          return GenPrefixedTypeName(WrapInNameSpace(*value.type.enum_def),
                                     value.type.enum_def->file) +
                 "." + val->name;
        } else {
          return WrapInNameSpace(*value.type.enum_def) + "." + val->name;
        }
      } else {
        return "/** @type {" + WrapInNameSpace(*value.type.enum_def) +
               "} */ (" + value.constant + ")";
      }
    }

    switch (value.type.base_type) {
      case BASE_TYPE_BOOL: return value.constant == "0" ? "false" : "true";

      case BASE_TYPE_STRING: return "null";

      case BASE_TYPE_LONG:
      case BASE_TYPE_ULONG: {
        int64_t constant = StringToInt(value.constant.c_str());
        return context + ".createLong(" + NumToString((int32_t)constant) +
               ", " + NumToString((int32_t)(constant >> 32)) + ")";
      }

      default: return value.constant;
    }
  }

  std::string GenTypeName(const Type &type, bool input,
                          bool allowNull = false) {
    if (!input) {
      if (type.base_type == BASE_TYPE_STRING ||
          type.base_type == BASE_TYPE_STRUCT) {
        std::string name;
        if (type.base_type == BASE_TYPE_STRING) {
          name = "string|Uint8Array";
        } else {
          name = WrapInNameSpace(*type.struct_def);
        }
        return (allowNull) ? (name + "|null") : (name);
      }
    }

    switch (type.base_type) {
      case BASE_TYPE_BOOL: return "boolean";
      case BASE_TYPE_LONG:
      case BASE_TYPE_ULONG: return "flatbuffers.Long";
      default:
        if (IsScalar(type.base_type)) {
          if (type.enum_def) { return WrapInNameSpace(*type.enum_def); }
          return "number";
        }
        return "flatbuffers.Offset";
    }
  }

  // Returns the method name for use with add/put calls.
  static std::string GenWriteMethod(const Type &type) {
    // Forward to signed versions since unsigned versions don't exist
    switch (type.base_type) {
      case BASE_TYPE_UTYPE:
      case BASE_TYPE_UCHAR: return GenWriteMethod(Type(BASE_TYPE_CHAR));
      case BASE_TYPE_USHORT: return GenWriteMethod(Type(BASE_TYPE_SHORT));
      case BASE_TYPE_UINT: return GenWriteMethod(Type(BASE_TYPE_INT));
      case BASE_TYPE_ULONG: return GenWriteMethod(Type(BASE_TYPE_LONG));
      default: break;
    }

    return IsScalar(type.base_type) ? MakeCamel(GenType(type))
                                    : (IsStruct(type) ? "Struct" : "Offset");
  }

  template<typename T> static std::string MaybeAdd(T value) {
    return value != 0 ? " + " + NumToString(value) : "";
  }

  template<typename T> static std::string MaybeScale(T value) {
    return value != 1 ? " * " + NumToString(value) : "";
  }

  static std::string GenFileNamespacePrefix(const std::string &file) {
    return "NS" + std::to_string(static_cast<unsigned long long>(
                      std::hash<std::string>()(file)));
  }

  static std::string GenPrefixedImport(const std::string &full_file_name,
                                       const std::string &base_file_name) {
    return "import * as " + GenFileNamespacePrefix(full_file_name) +
           " from \"./" + base_file_name + "\";\n";
  }

  // Adds a source-dependent prefix, for of import * statements.
  std::string GenPrefixedTypeName(const std::string &typeName,
                                  const std::string &file) {
    const auto basename =
        flatbuffers::StripPath(flatbuffers::StripExtension(file));
    if (basename == file_name_ || parser_.opts.generate_all) { return typeName; }
    return GenFileNamespacePrefix(file) + "." + typeName;
  }

  void GenStructArgs(const StructDef &struct_def, std::string *annotations,
                     std::string *arguments, const std::string &nameprefix) {
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (IsStruct(field.value.type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        GenStructArgs(*field.value.type.struct_def, annotations, arguments,
                      nameprefix + field.name + "_");
      } else {
        *annotations += "@param {" + GenTypeName(field.value.type, true);
        *annotations += "} " + nameprefix + field.name + "\n";

        if (lang_.language == IDLOptions::kTs) {
          *arguments += ", " + nameprefix + field.name + ": " +
                        GenTypeName(field.value.type, true);
        } else {
          *arguments += ", " + nameprefix + field.name;
        }
      }
    }
  }

  static void GenStructBody(const StructDef &struct_def, std::string *body,
                            const std::string &nameprefix) {
    *body += "  builder.prep(";
    *body += NumToString(struct_def.minalign) + ", ";
    *body += NumToString(struct_def.bytesize) + ");\n";

    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      if (field.padding) {
        *body += "  builder.pad(" + NumToString(field.padding) + ");\n";
      }
      if (IsStruct(field.value.type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        GenStructBody(*field.value.type.struct_def, body,
                      nameprefix + field.name + "_");
      } else {
        *body += "  builder.write" + GenWriteMethod(field.value.type) + "(";
        if (field.value.type.base_type == BASE_TYPE_BOOL) { *body += "+"; }
        *body += nameprefix + field.name + ");\n";
      }
    }
  }

  // Generate an accessor struct with constructor for a flatbuffers struct.
  void GenStruct(const Parser &parser, StructDef &struct_def,
                 std::string *code_ptr, std::string *exports_ptr,
                 imported_fileset &imported_files) {
    if (struct_def.generated) return;
    std::string &code = *code_ptr;
    std::string &exports = *exports_ptr;

    std::string object_name;
    std::string object_namespace = GetNameSpace(struct_def);

    // Emit constructor
    if (lang_.language == IDLOptions::kTs) {
      object_name = struct_def.name;
      GenDocComment(struct_def.doc_comment, code_ptr, "@constructor");
      if (!object_namespace.empty()) {
        code += "export namespace " + object_namespace + "{\n";
      }
      code += "export class " + struct_def.name;
      code += " {\n";
      code += "  /**\n";
      code += "   * @type {flatbuffers.ByteBuffer}\n";
      code += "   */\n";
      code += "  bb: flatbuffers.ByteBuffer|null = null;\n";
      code += "\n";
      code += "  /**\n";
      code += "   * @type {number}\n";
      code += "   */\n";
      code += "  bb_pos:number = 0;\n";
    } else {
      bool isStatement = struct_def.defined_namespace->components.empty();
      object_name = WrapInNameSpace(struct_def);
      GenDocComment(struct_def.doc_comment, code_ptr, "@constructor");
      if (isStatement) {
        if (parser_.opts.use_goog_js_export_format) {
          exports += "goog.exportSymbol('" + struct_def.name + "', " +
                     struct_def.name + ");\n";
        } else {
          exports +=
              "this." + struct_def.name + " = " + struct_def.name + ";\n";
        }
        code += "function " + object_name;
      } else {
        code += object_name + " = function";
      }
      code += "() {\n";
      code += "  /**\n";
      code += "   * @type {flatbuffers.ByteBuffer}\n";
      code += "   */\n";
      code += "  this.bb = null;\n";
      code += "\n";
      code += "  /**\n";
      code += "   * @type {number}\n";
      code += "   */\n";
      code += "  this.bb_pos = 0;\n";
      code += isStatement ? "}\n\n" : "};\n\n";
    }

    // Generate the __init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    code += "/**\n";
    code += " * @param {number} i\n";
    code += " * @param {flatbuffers.ByteBuffer} bb\n";
    code += " * @returns {" + object_name + "}\n";
    code += " */\n";

    if (lang_.language == IDLOptions::kTs) {
      code +=
          "__init(i:number, bb:flatbuffers.ByteBuffer):" + object_name + " {\n";
    } else {
      code += object_name + ".prototype.__init = function(i, bb) {\n";
    }

    code += "  this.bb_pos = i;\n";
    code += "  this.bb = bb;\n";
    code += "  return this;\n";
    code += "};\n\n";

    // Generate a special accessor for the table that when used as the root of a
    // FlatBuffer
    if (!struct_def.fixed) {
      GenDocComment(code_ptr,
                    "@param {flatbuffers.ByteBuffer} bb\n"
                    "@param {" +
                        object_name +
                        "=} obj\n"
                        "@returns {" +
                        object_name + "}");
      if (lang_.language == IDLOptions::kTs) {
        code += "static getRootAs" + struct_def.name;
        code += "(bb:flatbuffers.ByteBuffer, obj?:" + object_name +
                "):" + object_name + " {\n";
      } else {
        code += object_name + ".getRootAs" + struct_def.name;
        code += " = function(bb, obj) {\n";
      }
      code += "  return (obj || new " + object_name;
      code += ").__init(bb.readInt32(bb.position()) + bb.position(), bb);\n";
      code += "};\n\n";

      // Generate the identifier check method
      if (parser_.root_struct_def_ == &struct_def &&
          !parser_.file_identifier_.empty()) {
        GenDocComment(code_ptr,
                      "@param {flatbuffers.ByteBuffer} bb\n"
                      "@returns {boolean}");
        if (lang_.language == IDLOptions::kTs) {
          code +=
              "static bufferHasIdentifier(bb:flatbuffers.ByteBuffer):boolean "
              "{\n";
        } else {
          code += object_name + ".bufferHasIdentifier = function(bb) {\n";
        }

        code += "  return bb.__has_identifier('" + parser_.file_identifier_;
        code += "');\n};\n\n";
      }
    }

    // Emit field accessors
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto offset_prefix =
          "  var offset = " + GenBBAccess() + ".__offset(this.bb_pos, " +
          NumToString(field.value.offset) + ");\n  return offset ? ";

      // Emit a scalar field
      if (IsScalar(field.value.type.base_type) ||
          field.value.type.base_type == BASE_TYPE_STRING) {
        GenDocComment(
            field.doc_comment, code_ptr,
            std::string(
                field.value.type.base_type == BASE_TYPE_STRING
                    ? "@param {flatbuffers.Encoding=} optionalEncoding\n"
                    : "") +
                "@returns {" + GenTypeName(field.value.type, false, true) +
                "}");
        if (lang_.language == IDLOptions::kTs) {
          std::string prefix = MakeCamel(field.name, false) + "(";
          if (field.value.type.base_type == BASE_TYPE_STRING) {
            code += prefix + "):string|null\n";
            code += prefix + "optionalEncoding:flatbuffers.Encoding" +
                    "):" + GenTypeName(field.value.type, false, true) + "\n";
            code += prefix + "optionalEncoding?:any";
          } else {
            code += prefix;
          }
          if (field.value.type.enum_def) {
            code +=
                "):" +
                GenPrefixedTypeName(GenTypeName(field.value.type, false, true),
                                    field.value.type.enum_def->file) +
                " {\n";

            if (!parser_.opts.generate_all) {
                imported_files.insert(field.value.type.enum_def->file);
            }
          } else {
            code += "):" + GenTypeName(field.value.type, false, true) + " {\n";
          }
        } else {
          code += object_name + ".prototype." + MakeCamel(field.name, false);
          code += " = function(";
          if (field.value.type.base_type == BASE_TYPE_STRING) {
            code += "optionalEncoding";
          }
          code += ") {\n";
        }

        if (struct_def.fixed) {
          code +=
              "  return " +
              GenGetter(field.value.type,
                        "(this.bb_pos" + MaybeAdd(field.value.offset) + ")") +
              ";\n";
        } else {
          std::string index = "this.bb_pos + offset";
          if (field.value.type.base_type == BASE_TYPE_STRING) {
            index += ", optionalEncoding";
          }
          code += offset_prefix +
                  GenGetter(field.value.type, "(" + index + ")") + " : " +
                  GenDefaultValue(field.value, GenBBAccess());
          code += ";\n";
        }
      }

      // Emit an object field
      else {
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            auto type = WrapInNameSpace(*field.value.type.struct_def);
            GenDocComment(
                field.doc_comment, code_ptr,
                "@param {" + type + "=} obj\n@returns {" + type + "|null}");
            if (lang_.language == IDLOptions::kTs) {
              type =
                  GenPrefixedTypeName(type, field.value.type.struct_def->file);
              code += MakeCamel(field.name, false);
              code += "(obj?:" + type + "):" + type + "|null {\n";
            } else {
              code +=
                  object_name + ".prototype." + MakeCamel(field.name, false);
              code += " = function(obj) {\n";
            }

            if (struct_def.fixed) {
              code += "  return (obj || new " + type;
              code += ").__init(this.bb_pos";
              code +=
                  MaybeAdd(field.value.offset) + ", " + GenBBAccess() + ");\n";
            } else {
              code += offset_prefix + "(obj || new " + type + ").__init(";
              code += field.value.type.struct_def->fixed
                          ? "this.bb_pos + offset"
                          : GenBBAccess() + ".__indirect(this.bb_pos + offset)";
              code += ", " + GenBBAccess() + ") : null;\n";
            }

            if (lang_.language == IDLOptions::kTs && !parser_.opts.generate_all) {
              imported_files.insert(field.value.type.struct_def->file);
            }

            break;
          }

          case BASE_TYPE_VECTOR: {
            auto vectortype = field.value.type.VectorType();
            auto vectortypename = GenTypeName(vectortype, false);
            auto inline_size = InlineSize(vectortype);
            auto index = GenBBAccess() +
                         ".__vector(this.bb_pos + offset) + index" +
                         MaybeScale(inline_size);
            std::string args = "@param {number} index\n";
            std::string ret_type;
            bool is_union = false;
            switch (vectortype.base_type) {
              case BASE_TYPE_STRUCT:
                args += "@param {" + vectortypename + "=} obj\n";
                ret_type = vectortypename;
                break;
              case BASE_TYPE_STRING:
                args += "@param {flatbuffers.Encoding=} optionalEncoding\n";
                ret_type = vectortypename;
                break;
              case BASE_TYPE_UNION:
                args += "@param {flatbuffers.Table=} obj\n";
                ret_type = "?flatbuffers.Table";
                is_union = true;
                break;
              default: ret_type = vectortypename;
            }
            GenDocComment(field.doc_comment, code_ptr,
                          args + "@returns {" + ret_type + "}");
            if (lang_.language == IDLOptions::kTs) {
              std::string prefix = MakeCamel(field.name, false);
              if (is_union) { prefix += "<T extends flatbuffers.Table>"; }
              prefix += "(index: number";
              if (is_union) {
                vectortypename = "T";
                code += prefix + ", obj:T";
              } else if (vectortype.base_type == BASE_TYPE_STRUCT) {
                vectortypename = GenPrefixedTypeName(
                    vectortypename, vectortype.struct_def->file);
                code += prefix + ", obj?:" + vectortypename;

                if (!parser_.opts.generate_all) {
                    imported_files.insert(vectortype.struct_def->file);
                }
              } else if (vectortype.base_type == BASE_TYPE_STRING) {
                code += prefix + "):string\n";
                code += prefix + ",optionalEncoding:flatbuffers.Encoding" +
                        "):" + vectortypename + "\n";
                code += prefix + ",optionalEncoding?:any";
              } else {
                code += prefix;
              }
              code += "):" + vectortypename + "|null {\n";
            } else {
              code +=
                  object_name + ".prototype." + MakeCamel(field.name, false);
              code += " = function(index";
              if (vectortype.base_type == BASE_TYPE_STRUCT || is_union) {
                code += ", obj";
              } else if (vectortype.base_type == BASE_TYPE_STRING) {
                code += ", optionalEncoding";
              }
              code += ") {\n";
            }

            if (vectortype.base_type == BASE_TYPE_STRUCT) {
              code += offset_prefix + "(obj || new " + vectortypename;
              code += ").__init(";
              code += vectortype.struct_def->fixed
                          ? index
                          : GenBBAccess() + ".__indirect(" + index + ")";
              code += ", " + GenBBAccess() + ")";
            } else {
              if (is_union) {
                index = "obj, " + index;
              } else if (vectortype.base_type == BASE_TYPE_STRING) {
                index += ", optionalEncoding";
              }
              code += offset_prefix + GenGetter(vectortype, "(" + index + ")");
            }
            code += " : ";
            if (field.value.type.element == BASE_TYPE_BOOL) {
              code += "false";
            } else if (field.value.type.element == BASE_TYPE_LONG ||
                       field.value.type.element == BASE_TYPE_ULONG) {
              code += GenBBAccess() + ".createLong(0, 0)";
            } else if (IsScalar(field.value.type.element)) {
              if (field.value.type.enum_def) {
                code += "/** @type {" +
                        WrapInNameSpace(*field.value.type.enum_def) + "} */ (" +
                        field.value.constant + ")";
              } else {
                code += "0";
              }
            } else {
              code += "null";
            }
            code += ";\n";
            break;
          }

          case BASE_TYPE_UNION:
            GenDocComment(field.doc_comment, code_ptr,
                          "@param {flatbuffers.Table} obj\n"
                          "@returns {?flatbuffers.Table}");
            if (lang_.language == IDLOptions::kTs) {
              code += MakeCamel(field.name, false);
              code += "<T extends flatbuffers.Table>(obj:T):T|null {\n";
            } else {
              code +=
                  object_name + ".prototype." + MakeCamel(field.name, false);
              code += " = function(obj) {\n";
            }

            code += offset_prefix +
                    GenGetter(field.value.type, "(obj, this.bb_pos + offset)") +
                    " : null;\n";
            break;

          default: assert(0);
        }
      }
      code += "};\n\n";

      if (parser_.opts.use_goog_js_export_format) {
        exports += "goog.exportProperty(" + object_name + ".prototype, '" +
                   MakeCamel(field.name, false) + "', " + object_name +
                   ".prototype." + MakeCamel(field.name, false) + ");\n";
      }

      // Adds the mutable scalar value to the output
      if (IsScalar(field.value.type.base_type) && parser.opts.mutable_buffer) {
        std::string annotations =
            "@param {" + GenTypeName(field.value.type, true) + "} value\n";
        GenDocComment(code_ptr, annotations + "@returns {boolean}");

        if (lang_.language == IDLOptions::kTs) {
          std::string type;
          if (field.value.type.enum_def) {
            type = GenPrefixedTypeName(GenTypeName(field.value.type, true),
                                       field.value.type.enum_def->file);
          } else {
            type = GenTypeName(field.value.type, true);
          }

          code += "mutate_" + field.name + "(value:" + type + "):boolean {\n";
        } else {
          code += object_name + ".prototype.mutate_" + field.name +
                  " = function(value) {\n";
        }

        code += "  var offset = " + GenBBAccess() + ".__offset(this.bb_pos, " +
                NumToString(field.value.offset) + ");\n\n";
        code += "  if (offset === 0) {\n";
        code += "    return false;\n";
        code += "  }\n\n";

        // special case for bools, which are treated as uint8
        code += "  " + GenBBAccess() + ".write" +
                MakeCamel(GenType(field.value.type)) +
                "(this.bb_pos + offset, ";
        if (field.value.type.base_type == BASE_TYPE_BOOL &&
            lang_.language == IDLOptions::kTs) {
          code += "+";
        }

        code += "value);\n";
        code += "  return true;\n";
        code += "};\n\n";

        if (parser_.opts.use_goog_js_export_format) {
          exports += "goog.exportProperty(" + object_name +
                     ".prototype, 'mutate_" + field.name + "', " + object_name +
                     ".prototype.mutate_" + field.name + ");\n";
        }
      }

      // Emit vector helpers
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        // Emit a length helper
        GenDocComment(code_ptr, "@returns {number}");
        if (lang_.language == IDLOptions::kTs) {
          code += MakeCamel(field.name, false);
          code += "Length():number {\n" + offset_prefix;
        } else {
          code += object_name + ".prototype." + MakeCamel(field.name, false);
          code += "Length = function() {\n" + offset_prefix;
        }

        code +=
            GenBBAccess() + ".__vector_len(this.bb_pos + offset) : 0;\n};\n\n";

        if (parser_.opts.use_goog_js_export_format) {
          exports += "goog.exportProperty(" + object_name + ".prototype, '" +
                     MakeCamel(field.name, false) + "Length', " + object_name +
                     ".prototype." + MakeCamel(field.name, false) +
                     "Length);\n";
        }

        // For scalar types, emit a typed array helper
        auto vectorType = field.value.type.VectorType();
        if (IsScalar(vectorType.base_type) && !IsLong(vectorType.base_type)) {
          GenDocComment(code_ptr,
                        "@returns {" + GenType(vectorType) + "Array}");

          if (lang_.language == IDLOptions::kTs) {
            code += MakeCamel(field.name, false);
            code += "Array():" + GenType(vectorType) + "Array|null {\n" +
                    offset_prefix;
          } else {
            code += object_name + ".prototype." + MakeCamel(field.name, false);
            code += "Array = function() {\n" + offset_prefix;
          }

          code += "new " + GenType(vectorType) + "Array(" + GenBBAccess() +
                  ".bytes().buffer, " + GenBBAccess() +
                  ".bytes().byteOffset + " + GenBBAccess() +
                  ".__vector(this.bb_pos + offset), " + GenBBAccess() +
                  ".__vector_len(this.bb_pos + offset)) : null;\n};\n\n";

          if (parser_.opts.use_goog_js_export_format) {
            exports += "goog.exportProperty(" + object_name + ".prototype, '" +
                       MakeCamel(field.name, false) + "Array', " + object_name +
                       ".prototype." + MakeCamel(field.name, false) +
                       "Array);\n";
          }
        }
      }
    }

    // Emit a factory constructor
    if (struct_def.fixed) {
      std::string annotations = "@param {flatbuffers.Builder} builder\n";
      std::string arguments;
      GenStructArgs(struct_def, &annotations, &arguments, "");
      GenDocComment(code_ptr, annotations + "@returns {flatbuffers.Offset}");

      if (lang_.language == IDLOptions::kTs) {
        code +=
            "static create" + struct_def.name + "(builder:flatbuffers.Builder";
        code += arguments + "):flatbuffers.Offset {\n";
      } else {
        code +=
            object_name + ".create" + struct_def.name + " = function(builder";
        code += arguments + ") {\n";
      }

      GenStructBody(struct_def, &code, "");
      code += "  return builder.offset();\n};\n\n";
    } else {
      // Generate a method to start building a new object
      GenDocComment(code_ptr, "@param {flatbuffers.Builder} builder");

      if (lang_.language == IDLOptions::kTs) {
        code += "static start" + struct_def.name;
        code += "(builder:flatbuffers.Builder) {\n";
      } else {
        code += object_name + ".start" + struct_def.name;
        code += " = function(builder) {\n";
      }

      code += "  builder.startObject(" +
              NumToString(struct_def.fields.vec.size()) + ");\n";
      code += "};\n\n";

      // Generate a set of static methods that allow table construction
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        auto argname = MakeCamel(field.name, false);
        if (!IsScalar(field.value.type.base_type)) { argname += "Offset"; }

        // Generate the field insertion method
        GenDocComment(code_ptr,
                      "@param {flatbuffers.Builder} builder\n"
                      "@param {" +
                          GenTypeName(field.value.type, true) + "} " + argname);

        if (lang_.language == IDLOptions::kTs) {
          std::string argType;
          if (field.value.type.enum_def) {
            argType = GenPrefixedTypeName(GenTypeName(field.value.type, true),
                                          field.value.type.enum_def->file);
          } else {
            argType = GenTypeName(field.value.type, true);
          }

          code += "static add" + MakeCamel(field.name);
          code += "(builder:flatbuffers.Builder, " + argname + ":" + argType +
                  ") {\n";
        } else {
          code += object_name + ".add" + MakeCamel(field.name);
          code += " = function(builder, " + argname + ") {\n";
        }

        code += "  builder.addField" + GenWriteMethod(field.value.type) + "(";
        code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
        if (field.value.type.base_type == BASE_TYPE_BOOL) { code += "+"; }
        code += argname + ", ";
        if (!IsScalar(field.value.type.base_type)) {
          code += "0";
        } else {
          if (field.value.type.base_type == BASE_TYPE_BOOL) { code += "+"; }
          code += GenDefaultValue(field.value, "builder");
        }
        code += ");\n};\n\n";

        if (field.value.type.base_type == BASE_TYPE_VECTOR) {
          auto vector_type = field.value.type.VectorType();
          auto alignment = InlineAlignment(vector_type);
          auto elem_size = InlineSize(vector_type);

          // Generate a method to create a vector from a JavaScript array
          if (!IsStruct(vector_type)) {
            GenDocComment(code_ptr,
                          "@param {flatbuffers.Builder} builder\n"
                          "@param {Array.<" +
                              GenTypeName(vector_type, true) +
                              ">} data\n"
                              "@returns {flatbuffers.Offset}");

            if (lang_.language == IDLOptions::kTs) {
              code += "static create" + MakeCamel(field.name);
              std::string type = GenTypeName(vector_type, true) + "[]";
              if (type == "number[]") { type += " | Uint8Array"; }
              code += "Vector(builder:flatbuffers.Builder, data:" + type +
                      "):flatbuffers.Offset {\n";
            } else {
              code += object_name + ".create" + MakeCamel(field.name);
              code += "Vector = function(builder, data) {\n";
            }

            code += "  builder.startVector(" + NumToString(elem_size);
            code += ", data.length, " + NumToString(alignment) + ");\n";
            code += "  for (var i = data.length - 1; i >= 0; i--) {\n";
            code += "    builder.add" + GenWriteMethod(vector_type) + "(";
            if (vector_type.base_type == BASE_TYPE_BOOL) { code += "+"; }
            code += "data[i]);\n";
            code += "  }\n";
            code += "  return builder.endVector();\n";
            code += "};\n\n";
          }

          // Generate a method to start a vector, data to be added manually
          // after
          GenDocComment(code_ptr,
                        "@param {flatbuffers.Builder} builder\n"
                        "@param {number} numElems");

          if (lang_.language == IDLOptions::kTs) {
            code += "static start" + MakeCamel(field.name);
            code += "Vector(builder:flatbuffers.Builder, numElems:number) {\n";
          } else {
            code += object_name + ".start" + MakeCamel(field.name);
            code += "Vector = function(builder, numElems) {\n";
          }

          code += "  builder.startVector(" + NumToString(elem_size);
          code += ", numElems, " + NumToString(alignment) + ");\n";
          code += "};\n\n";
        }
      }

      // Generate a method to stop building a new object
      GenDocComment(code_ptr,
                    "@param {flatbuffers.Builder} builder\n"
                    "@returns {flatbuffers.Offset}");

      if (lang_.language == IDLOptions::kTs) {
        code += "static end" + struct_def.name;
        code += "(builder:flatbuffers.Builder):flatbuffers.Offset {\n";
      } else {
        code += object_name + ".end" + struct_def.name;
        code += " = function(builder) {\n";
      }

      code += "  var offset = builder.endObject();\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated && field.required) {
          code += "  builder.requiredField(offset, ";
          code += NumToString(field.value.offset);
          code += "); // " + field.name + "\n";
        }
      }
      code += "  return offset;\n";
      code += "};\n\n";

      // Generate the method to complete buffer construction
      if (parser_.root_struct_def_ == &struct_def) {
        GenDocComment(code_ptr,
                      "@param {flatbuffers.Builder} builder\n"
                      "@param {flatbuffers.Offset} offset");

        if (lang_.language == IDLOptions::kTs) {
          code += "static finish" + struct_def.name + "Buffer";
          code +=
              "(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {\n";
        } else {
          code += object_name + ".finish" + struct_def.name + "Buffer";
          code += " = function(builder, offset) {\n";
        }

        code += "  builder.finish(offset";
        if (!parser_.file_identifier_.empty()) {
          code += ", '" + parser_.file_identifier_ + "'";
        }
        code += ");\n";
        code += "};\n\n";
      }
    }

    if (lang_.language == IDLOptions::kTs) {
      if (!object_namespace.empty()) { code += "}\n"; }
      code += "}\n";
    }
  }
};
}  // namespace js

bool GenerateJS(const Parser &parser, const std::string &path,
                const std::string &file_name) {
  js::JsGenerator generator(parser, path, file_name);
  return generator.generate();
}

std::string JSMakeRule(const Parser &parser, const std::string &path,
                       const std::string &file_name) {
  assert(parser.opts.lang <= IDLOptions::kMAX);
  const auto &lang = GetJsLangParams(parser.opts.lang);

  std::string filebase =
      flatbuffers::StripPath(flatbuffers::StripExtension(file_name));
  std::string make_rule = GeneratedFileName(path, filebase, lang) + ": ";

  auto included_files = parser.GetIncludedFilesRecursive(file_name);
  for (auto it = included_files.begin(); it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers
