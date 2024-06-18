NET_VERSION=mobilenet_v2_1.0_96
DATASET_DIR=${PWD}/data/trafficsigns
TRAIN_DIR=${PWD}/data/trained-model
PRETRAINED_CHECKPOINT_DIR=${PWD}/data/pretrained-models/${NET_VERSION}
cd slim/
python train_image_classifier.py \
  --train_dir=${TRAIN_DIR} \
  --dataset_name=trafficsigns \
  --dataset_split_name=train \
  --dataset_dir=${DATASET_DIR} \
  --clone_on_cpu=True \
  --model_name=mobilenet_v2 \
  --train_image_size=96 \
  --checkpoint_path=${PRETRAINED_CHECKPOINT_DIR}/${NET_VERSION}.ckpt \
  --checkpoint_exclude_scopes=MobilenetV2/Logits,MobilenetV2/Predictions,MobilenetV2/predics \
  --trainable_scopes=MobilenetV2/Logits,MobilenetV2/Predictions,MobilenetV2/predics \
  --max_number_of_steps=1600\
  --batch_size=32 \
  --learning_rate=0.001 \
  --learning_rate_decay_type=fixed \
  --save_interval_secs=60 \
  --save_summaries_secs=60 \
  --log_every_n_steps=100 \
  --optimizer=rmsprop \
  --weight_decay=0.00004

  #--checkpoint_exclude_scopes=MobilenetV2/Logits,MobilenetV2/AuxLogits \
  #--trainable_scopes=MobilenetV2/Logits,MobilenetV2/AuxLogits \
