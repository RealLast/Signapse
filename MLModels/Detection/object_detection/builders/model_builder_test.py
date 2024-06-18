# Copyright 2017 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

"""Tests for object_detection.models.model_builder."""

import tensorflow as tf

from google.protobuf import text_format
from object_detection.builders import model_builder
from object_detection.meta_architectures import ssd_meta_arch
from object_detection.models.ssd_mobilenet_v2_feature_extractor import SSDMobileNetV2FeatureExtractor
from object_detection.protos import model_pb2


class ModelBuilderTest(tf.test.TestCase):

    def create_model(self, model_config):
        """Builds a DetectionModel based on the model config.

        Args:
          model_config: A model.proto object containing the config for the desired
            DetectionModel.

        Returns:
          DetectionModel based on the config.
        """
        return model_builder.build(model_config, is_training=True)

    def test_create_ssd_mobilenet_v2_model_from_config(self):
        model_text_proto = """
      ssd {
        feature_extractor {
          type: 'ssd_mobilenet_v2'
          conv_hyperparams {
            regularizer {
                l2_regularizer {
                }
              }
              initializer {
                truncated_normal_initializer {
                }
              }
          }
        }
        box_coder {
          faster_rcnn_box_coder {
          }
        }
        matcher {
          argmax_matcher {
          }
        }
        similarity_calculator {
          iou_similarity {
          }
        }
        anchor_generator {
          ssd_anchor_generator {
            aspect_ratios: 1.0
          }
        }
        image_resizer {
          fixed_shape_resizer {
            height: 320
            width: 320
          }
        }
        box_predictor {
          convolutional_box_predictor {
            conv_hyperparams {
              regularizer {
                l2_regularizer {
                }
              }
              initializer {
                truncated_normal_initializer {
                }
              }
            }
          }
        }
        normalize_loc_loss_by_codesize: true
        loss {
          classification_loss {
            weighted_softmax {
            }
          }
          localization_loss {
            weighted_smooth_l1 {
            }
          }
        }
      }"""
        model_proto = model_pb2.DetectionModel()
        text_format.Merge(model_text_proto, model_proto)
        model = self.create_model(model_proto)
        self.assertIsInstance(model, ssd_meta_arch.SSDMetaArch)
        self.assertIsInstance(model._feature_extractor,
                              SSDMobileNetV2FeatureExtractor)
        self.assertTrue(model._normalize_loc_loss_by_codesize)


if __name__ == '__main__':
    tf.test.main()
