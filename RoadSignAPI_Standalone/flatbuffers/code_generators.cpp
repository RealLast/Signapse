/*
 * Copyright 2016 Google Inc. All rights reserved.
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

#include "flatbuffers/code_generators.h"
#include <assert.h>
#include "flatbuffers/base.h"
#include "flatbuffers/util.h"

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4127)  // C4127: conditional expression is constant
#endif

namespace flatbuffers {

void CodeWriter::operator+=(std::string text) {
  while (true) {
    auto begin = text.find("{{");
    if (begin == std::string::npos) { break; }

    auto end = text.find("}}");
    if (end == std::string::npos || end < begin) { break; }

    // Write all the text before the first {{ into the stream.
    stream_.write(text.c_str(), begin);

    // The key is between the {{ and }}.
    const std::string key = text.substr(begin + 2, end - begin - 2);

    // Find the value associated with the key.  If it exists, write the
    // value into the stream, otherwise write the key itself into the stream.
    auto iter = value_map_.find(key);
    if (iter != value_map_.end()) {
      const std::string &value = iter->second;
      stream_ << value;
    } else {
      assert(false && "could not find key");
      stream_ << key;
    }

    // Update the text to everything after the }}.
    text = text.substr(end + 2);
  }
  if (!text.empty() && string_back(text) == '\\') {
    text.pop_back();
    stream_ << text;
  } else {
    stream_ << text << std::endl;
  }
}

const char *BaseGenerator::FlatBuffersGeneratedWarning() {
  return "automatically generated by the FlatBuffers compiler,"
         " do not modify";
}

std::string BaseGenerator::NamespaceDir(const Parser &parser,
                                        const std::string &path,
                                        const Namespace &ns) {
  EnsureDirExists(path.c_str());
  if (parser.opts.one_file) return path;
  std::string namespace_dir = path;  // Either empty or ends in separator.
  auto &namespaces = ns.components;
  for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
    namespace_dir += *it + kPathSeparator;
    EnsureDirExists(namespace_dir.c_str());
  }
  return namespace_dir;
}

std::string BaseGenerator::NamespaceDir(const Namespace &ns) const {
  return BaseGenerator::NamespaceDir(parser_, path_, ns);
}

std::string BaseGenerator::FullNamespace(const char *separator,
                                         const Namespace &ns) {
  std::string namespace_name;
  auto &namespaces = ns.components;
  for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
    if (namespace_name.length()) namespace_name += separator;
    namespace_name += *it;
  }
  return namespace_name;
}

std::string BaseGenerator::LastNamespacePart(const Namespace &ns) {
  if (!ns.components.empty())
    return ns.components.back();
  else
    return std::string("");
}

// Ensure that a type is prefixed with its namespace whenever it is used
// outside of its namespace.
std::string BaseGenerator::WrapInNameSpace(const Namespace *ns,
                                           const std::string &name) const {
  if (CurrentNameSpace() == ns) return name;
  std::string qualified_name = qualifying_start_;
  for (auto it = ns->components.begin(); it != ns->components.end(); ++it)
    qualified_name += *it + qualifying_separator_;
  return qualified_name + name;
}

std::string BaseGenerator::WrapInNameSpace(const Definition &def) const {
  return WrapInNameSpace(def.defined_namespace, def.name);
}

std::string BaseGenerator::GetNameSpace(const Definition &def) const {
  const Namespace *ns = def.defined_namespace;
  if (CurrentNameSpace() == ns) return "";
  std::string qualified_name = qualifying_start_;
  for (auto it = ns->components.begin(); it != ns->components.end(); ++it) {
    qualified_name += *it;
    if ((it + 1) != ns->components.end()) {
      qualified_name += qualifying_separator_;
    }
  }

  return qualified_name;
}

// Generate a documentation comment, if available.
void GenComment(const std::vector<std::string> &dc, std::string *code_ptr,
                const CommentConfig *config, const char *prefix) {
  if (dc.begin() == dc.end()) {
    // Don't output empty comment blocks with 0 lines of comment content.
    return;
  }

  std::string &code = *code_ptr;
  if (config != nullptr && config->first_line != nullptr) {
    code += std::string(prefix) + std::string(config->first_line) + "\n";
  }
  std::string line_prefix =
      std::string(prefix) +
      ((config != nullptr && config->content_line_prefix != nullptr)
           ? config->content_line_prefix
           : "///");
  for (auto it = dc.begin(); it != dc.end(); ++it) {
    code += line_prefix + *it + "\n";
  }
  if (config != nullptr && config->last_line != nullptr) {
    code += std::string(prefix) + std::string(config->last_line) + "\n";
  }
}

}  // namespace flatbuffers

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
