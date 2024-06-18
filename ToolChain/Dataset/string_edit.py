###############################################################################
# usage:   Converts LabelImg multifile XML output into single csv file        #
# author:  Robert Niklas Bock                                                 #
###############################################################################


def add_str_to_lines(f_name, str_to_add):
    with open(f_name, "r") as f:
        lines = f.readlines()
        for index, line in enumerate(lines):
            lines[index] = line.strip() + str_to_add + "\n"

    with open(f_name, "w") as f:
        for line in lines:
            f.write(line)


if __name__ == "__main__":
    STRING_TO_ADD = ",0,0,0"
    FILENAME = "labels.csv"
    add_str_to_lines(f_name=FILENAME, str_to_add=STRING_TO_ADD)
