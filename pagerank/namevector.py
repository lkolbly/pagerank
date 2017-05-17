name_f = open("name-reference")
vec_f = open("vector-out")
while 1:
    name = name_f.readline().strip(" \r\n").replace(",", "\,")
    val = vec_f.readline().strip(" \r\n")
    if len(name) == 0:
        break
    print "%s,%s"%(name,val)
    pass
vec_f.close()
name_f.close()
