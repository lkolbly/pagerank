import os, gzip

pageviews = {}

toview = sorted(os.listdir("pagecounts"))
for fname in toview:
    try:
        print "Processing %s..."%fname
        f = gzip.open("pagecounts/%s"%fname)
        for line in f:
            parts = line.strip(" \r\n").split(" ")
            if len(parts) == 0:
                continue
            if parts[0] != "en":
                continue
            pageviews[parts[1]] = pageviews.get(parts[1], 0) + int(parts[2])
        f.close()
    except:
        print "Error processing %s"%fname
    pass

pageviews2 = []
for k,v in pageviews.items():
    pageviews2.append((k,v))
pageviews2 = sorted(pageviews2, key=lambda v: -v[1])

f = open("total-pagecounts", "w")
for v in pageviews2:
    f.write("%s %s\n"%v)
f.close()
