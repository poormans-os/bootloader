from sys import version_info
if version_info.major != 3:
    raise(EnvironmentError("UNSUPPORTED VERSION"))

from argparse import ArgumentParser, RawDescriptionHelpFormatter
import os.path

bfToPmos = {'>': "poor", '<': "mans", '+': "operating",
            '-': "system", '[': "pmos", ']': "a", ',': "b", '.': "c"}

pmosToBf = {"poor": '>',  "mans": '<',  "operating": '+',
            "system": '-', "pmos": '[', "a": ']', "b": ',',  "c": '.'}

srcfile = ""
destfile = ""


def switchLang(keyword: dict, pmos=False) -> None:
    with open(srcfile, 'r') as f:
        data = f.read()

    content = ""

    if pmos:
        data = data.split(' ')

    for char in data:
        if char in keyword:
            content = content + " " + keyword[char]
            # print(keyword[char], end=" ")
    content = content[1:]

    with open(destfile, 'w') as f:
        f.write(content)


def main() -> None:
    global srcfile, destfile

    prs = ArgumentParser(prog="Convert", formatter_class=RawDescriptionHelpFormatter,
                         description=("Convert From BF to PMOS-LANG"))
    prs.add_argument("dest", help="The Destination File",
                     type=str)
    prs.add_argument("src", help="The Source File",
                     type=str)
    prs.add_argument("-e", help="From BF To PMOS, Thats The default",
                     action='store_true', dest="encrypt", required=False)
    prs.add_argument("-d", help="From PMOS To BF",
                     action='store_true', dest="decrypt", required=False)
    args = prs.parse_args()

    print(args)

    srcfile = args.src
    destfile = args.dest

    if not os.path.isfile(srcfile):
        raise("Source File Doesn't Exist")

    if args.decrypt:
        switchLang(pmosToBf, True)
    else:
        switchLang(bfToPmos)


if __name__ == '__main__':
    main()
