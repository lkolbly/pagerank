import sys

pages = []

for line in sys.stdin.xreadlines():
    line = line.strip(" \r\n").replace("\,", " ")
    if len(line) == 0:
        continue
    line = line.split(",")
    if len(line) < 2:
        continue
    try:
        pages.append((line[0],float(line[1])))
    except:
        pass
    pass

pages = sorted(pages, key=lambda v: -v[1])
for p in pages:
    print "%s,%.12e"%p
