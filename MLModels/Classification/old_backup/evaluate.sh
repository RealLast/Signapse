DATASET_DIR=${PWD}/data/trafficsigns
TRAIN_DIR=${PWD}/data/trained-model
cd slim/
python eval_image_classifier.py \
  --checkpoint_path=${TRAIN_DIR} \
  --eval_dir=${TRAIN_DIR} \
  --dataset_name=trafficsigns \
  --dataset_split_name=validation \
  --dataset_dir=${DATASET_DIR} \
  --model_name=mobilenet_v2 \
  --eval_image_size=96
