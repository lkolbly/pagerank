f_actual = open("../pagecounts/total-pagecounts")

pages = {}

cnt = 1
for line in f_actual:
    parts = line.strip(" \r\n").split(" ")
    #pages.append((parts[0], int(parts[1])))
    pages[parts[0]] = (cnt,int(parts[1]))
    cnt+=1
    pass

f_pagerank = open("pageranked")

cnt = 1
print "Article Name,Pagerank's rank,Actual rank"
for line in f_pagerank:
    parts = line.strip(" \r\n").split(",")
    parts[0] = parts[0].strip(" /").replace(" ","_")
    if parts[0] in pages:
        print "%s,%s,%s"%(parts[0], cnt, pages[parts[0]][0])
    else:
        print "%s,%s,%s"%(parts[0], cnt, -1)
    cnt += 1

f_actual.close()
f_pagerank.close()
