import math

MAXVAL = 16000000 # Square
NBUCKETS = 512

buckets = []#[0]*NBUCKETS]*NBUCKETS
for x in xrange(NBUCKETS):
    l = []
    for y in xrange(NBUCKETS):
        l.append(0)
    buckets.append(l)

points = []
f = open("results.csv")
f.readline() # Skip header
for line in f:
    line = line.strip(" \r\n")
    if len(line) == 0:
        continue
    parts = line.split(",")
    x = int(parts[1])
    y = int(parts[2])
    if y > -1:
        points.append((x,y))
f.close()

# Rescale the empirical ranking to be on the same scale
points = sorted(points, key=lambda v: v[1])
cnt = 1
for i in xrange(len(points)):
    points[i] = (points[i][0], cnt)
    print points[i]
    cnt += 1

for x,y in points:
    x = x * NBUCKETS / MAXVAL
    y = y * NBUCKETS / MAXVAL
    try:
        buckets[int(x)][int(y)] += 1
    except:
        print x,y


minimum = buckets[0][0]
maximum = buckets[0][0]

f = open("histogram.dat", "w")
for x in xrange(NBUCKETS):
    for y in xrange(1,NBUCKETS-1):
        f.write("%s\t%s\t%s\n"%(x,y,math.log(1+buckets[x][y])))
    f.write("\n")
f.close()
