import numpy as np
import cv2
import pandas as pd
import argparse
import matplotlib.pyplot as pyplot
import os


THRESHOLD = 0.5
THRESHOLD_CONFIDENCE = 0.5


def get_iou_rect(rect_gt, rect_d):
    its_x_min = max(rect_gt[0], rect_d[0])
    its_x_max = min(rect_gt[2], rect_d[2])
    its_y_min = max(rect_gt[1], rect_d[1])
    its_y_max = min(rect_gt[3], rect_d[3])
    gt_area = (rect_gt[2] - rect_gt[0]) * (rect_gt[3] - rect_gt[1])
    d_area = (rect_d[2] - rect_d[0]) * (rect_d[3] - rect_d[1])
    if gt_area > 0 and d_area > 0:
        its_area = (its_x_max-its_x_min) * (its_y_max-its_y_min)
    else:
        its_area = 0
    iou = float(its_area)/(gt_area + d_area - its_area)
    return iou


def calc_recall(true_positive, false_negative):
    return float(true_positive)/(true_positive + false_negative)


def calc_precision(true_positive, false_positive):
    return float(true_positive)/(true_positive + false_positive)


def calc_f1_score(precision, recall):
    if not recall == 0 and not precision == 0:
        f1_score = 2 * precision * recall/(precision + recall)
    else:
        f1_score = 0
    return f1_score


def get_gt_vs_det(gt, d):
    if len(d) == 0:
        return (0, 0, 1)
    true_positive = 0
    false_positive = 0
    false_negative = 0

    result = np.empty([len(gt), len(d)])
    for i in range(0, len(gt)):
        for j in range(0, len(d)):
            score = get_iou_rect(gt[i], d[j])
            result[[i], [j]] = score

    for i in range(0, len(result)):
        max_in_row = np.argmax(result[i])
        if result[i][max_in_row] > THRESHOLD:
            true_positive += 1
        elif result[i][max_in_row] == 0:
            false_negative += 1
        else:
            false_positive += 1
            false_negative += 1
    return (true_positive, false_positive, false_negative)


def calc_f1(gt_labels_df, det_labels_df, threshold_con):
    tp_cache = 0
    fp_cache = 0
    fn_cache = 0

    for row in gt_labels_df.iterrows():
        _, data = row
        gt_box = [[data.ix['xmin'], data.ix['ymin'], data.ix['xmax'], data.ix['ymax']]]
        det_df = det_labels_df.loc[det_labels_df['filename'] == data['filename']]

        det_box = np.empty([len(det_df), 4])
        counter = 0
        for row_det in det_df.iterrows():
            _, data_det = row_det
            if data_det['confidence'] > threshold_con:
                det_box[counter] = [data_det['xmin'], data_det['ymin'], data_det['xmax'], data_det['ymax']]
            counter += 1
        (true_positive, false_positive, false_negative) = get_gt_vs_det(gt_box, det_box)
        tp_cache += true_positive
        fp_cache += false_positive
        fn_cache += false_negative

    recall = calc_recall(tp_cache, fn_cache)
    precision = calc_precision(tp_cache, fp_cache)
    f1 = calc_f1_score(recall, precision)
    return (f1, recall, precision)


def main():
    gt_labels_df = pd.read_csv("gt.csv")
    results = np.empty([101])
    counter = 0
    # for filename in os.listdir("labels"):
    if 1 == 1:
        det_labels_df = pd.read_csv(os.path.join("labels", "553273labels.csv"))
        y = np.empty([101])
        x = np.empty([101])
        for i in range(0, 101):
            (f1, recall, precision) = calc_f1(gt_labels_df, det_labels_df, i*0.01)
            # print(str(f1))
            # print(i*0.01)
            y[i] = f1
            x[i] = i*0.01
        f1_max = np.amax(y)
        f1_max_theshold = x[np.argmax(y)]
        print("F1: " + str(f1_max))
        print("Theshold: " + str(f1_max_theshold))
        pyplot.plot(x, y)
        pyplot.xlabel("Threshold Detector")
        pyplot.ylabel("F1-Score")
        pyplot.show()
        print("Recall: " + str(recall))
        print("Precision: " + str(precision))
        print("F1-Score: " + str(f1))
        results[counter] = f1_max
        counter += 1
    print("F1 Max is: " + str(np.amax(results)))
    print("At pos: " + str(np.argmax(results)))


if __name__ == "__main__":
    main()
