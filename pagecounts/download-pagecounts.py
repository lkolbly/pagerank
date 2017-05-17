import urllib

for day in range(1,31):
    for hour in range(24):
        url = "https://dumps.wikimedia.org/other/pagecounts-raw/2014/2014-11/pagecounts-201411%02i-%02i0000.gz"%(day,hour)
        print url
        urllib.urlretrieve(url, "pagecounts/201411%02i%02i.gz"%(day,hour))
        pass
    pass
