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

"""A function to build a DetectionModel from configuration."""
from object_detection.builders import anchor_generator_builder
from object_detection.builders import box_coder_builder
from object_detection.builders import box_predictor_builder
from object_detection.builders import hyperparams_builder
from object_detection.builders import image_resizer_builder
from object_detection.builders import losses_builder
from object_detection.builders import matcher_builder
from object_detection.builders import post_processing_builder
from object_detection.builders import region_similarity_calculator_builder as sim_calc
from object_detection.core import box_predictor
from object_detection.meta_architectures import ssd_meta_arch
from object_detection.models.ssd_mobilenet_v2_feature_extractor import SSDMobileNetV2FeatureExtractor
from object_detection.protos import model_pb2

# A map of names to SSD feature extractors.
SSD_FEATURE_EXTRACTOR_CLASS_MAP = {
    'ssd_mobilenet_v2': SSDMobileNetV2FeatureExtractor,
}


def build(model_config, is_training, add_summaries=True,
          add_background_class=True):
    """Builds a DetectionModel based on the model config.

    Args:
      model_config: A model.proto object containing the config for the desired
        DetectionModel.
      is_training: True if this model is being built for training purposes.
      add_summaries: Whether to add tensorflow summaries in the model graph.
      add_background_class: Whether to add an implicit background class to one-hot
        encodings of groundtruth labels. Set to false if using groundtruth labels
        with an explicit background class or using multiclass scores instead of
        truth in the case of distillation. Ignored in the case of faster_rcnn.
    Returns:
      DetectionModel based on the config.

    Raises:
      ValueError: On invalid meta architecture or model.
    """
    if not isinstance(model_config, model_pb2.DetectionModel):
        raise ValueError('model_config not of type model_pb2.DetectionModel.')
    meta_architecture = model_config.WhichOneof('model')
    if meta_architecture == 'ssd':
        return _build_ssd_model(model_config.ssd, is_training, add_summaries,
                                add_background_class)
    raise ValueError('Unknown meta architecture: {}'.format(meta_architecture))


def _build_ssd_feature_extractor(feature_extractor_config, is_training,
                                 reuse_weights=None):
    """Builds a ssd_meta_arch.SSDFeatureExtractor based on config.

    Args:
      feature_extractor_config: A SSDFeatureExtractor proto config from ssd.proto.
      is_training: True if this feature extractor is being built for training.
      reuse_weights: if the feature extractor should reuse weights.

    Returns:
      ssd_meta_arch.SSDFeatureExtractor based on config.

    Raises:
      ValueError: On invalid feature extractor type.
    """
    feature_type = feature_extractor_config.type
    depth_multiplier = feature_extractor_config.depth_multiplier
    min_depth = feature_extractor_config.min_depth
    pad_to_multiple = feature_extractor_config.pad_to_multiple
    use_explicit_padding = feature_extractor_config.use_explicit_padding
    use_depthwise = feature_extractor_config.use_depthwise
    conv_hyperparams = hyperparams_builder.build(
        feature_extractor_config.conv_hyperparams, is_training)
    override_base_feature_extractor_hyperparams = (
        feature_extractor_config.override_base_feature_extractor_hyperparams)

    if feature_type not in SSD_FEATURE_EXTRACTOR_CLASS_MAP:
        raise ValueError('Unknown ssd feature_extractor: {}'.format(feature_type))

    feature_extractor_class = SSD_FEATURE_EXTRACTOR_CLASS_MAP[feature_type]
    return feature_extractor_class(
        is_training, depth_multiplier, min_depth, pad_to_multiple,
        conv_hyperparams, reuse_weights, use_explicit_padding, use_depthwise,
        override_base_feature_extractor_hyperparams)


def _build_ssd_model(ssd_config, is_training, add_summaries,
                     add_background_class=True):
    """Builds an SSD detection model based on the model config.

    Args:
      ssd_config: A ssd.proto object containing the config for the desired
        SSDMetaArch.
      is_training: True if this model is being built for training purposes.
      add_summaries: Whether to add tf summaries in the model.
      add_background_class: Whether to add an implicit background class to one-hot
        encodings of groundtruth labels. Set to false if using groundtruth labels
        with an explicit background class or using multiclass scores instead of
        truth in the case of distillation.
    Returns:
      SSDMetaArch based on the config.

    Raises:
      ValueError: If ssd_config.type is not recognized (i.e. not registered in
        model_class_map).
    """
    num_classes = ssd_config.num_classes

    # Feature extractor
    feature_extractor = _build_ssd_feature_extractor(
        feature_extractor_config=ssd_config.feature_extractor,
        is_training=is_training)

    box_coder = box_coder_builder.build(ssd_config.box_coder)
    matcher = matcher_builder.build(ssd_config.matcher)
    region_similarity_calculator = sim_calc.build(
        ssd_config.similarity_calculator)
    encode_background_as_zeros = ssd_config.encode_background_as_zeros
    negative_class_weight = ssd_config.negative_class_weight
    ssd_box_predictor = box_predictor_builder.build(hyperparams_builder.build,
                                                    ssd_config.box_predictor,
                                                    is_training, num_classes)
    anchor_generator = anchor_generator_builder.build(
        ssd_config.anchor_generator)
    image_resizer_fn = image_resizer_builder.build(ssd_config.image_resizer)
    non_max_suppression_fn, score_conversion_fn = post_processing_builder.build(
        ssd_config.post_processing)
    (classification_loss, localization_loss, classification_weight,
     localization_weight, hard_example_miner,
     random_example_sampler) = losses_builder.build(ssd_config.loss)
    normalize_loss_by_num_matches = ssd_config.normalize_loss_by_num_matches
    normalize_loc_loss_by_codesize = ssd_config.normalize_loc_loss_by_codesize

    return ssd_meta_arch.SSDMetaArch(
        is_training,
        anchor_generator,
        ssd_box_predictor,
        box_coder,
        feature_extractor,
        matcher,
        region_similarity_calculator,
        encode_background_as_zeros,
        negative_class_weight,
        image_resizer_fn,
        non_max_suppression_fn,
        score_conversion_fn,
        classification_loss,
        localization_loss,
        classification_weight,
        localization_weight,
        normalize_loss_by_num_matches,
        hard_example_miner,
        add_summaries=add_summaries,
        normalize_loc_loss_by_codesize=normalize_loc_loss_by_codesize,
        freeze_batchnorm=ssd_config.freeze_batchnorm,
        inplace_batchnorm_update=ssd_config.inplace_batchnorm_update,
        add_background_class=add_background_class,
        random_example_sampler=random_example_sampler)
