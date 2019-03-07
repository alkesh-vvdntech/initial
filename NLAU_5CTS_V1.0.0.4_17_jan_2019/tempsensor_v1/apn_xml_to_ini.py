#!/usr/bin/python

from xml.dom import minidom

filter = False

x = minidom.parse('apn-full-conf.xml')
apns = x.getElementsByTagName('apn')
full_apns = {}
IN_mcc = [404, 405]
US_mcc = [310, 311, 316]

exclude_list_IN = ['mms', 'mizone', 'airtelfun.com', 'imis', 'aircelwebpost', 'aircelwappost', 'aircelwap', 'aircelweb', 'bsnllive', \
                'myaircelpost', 'myaircel', 'aircelgprs.po', 'aircelgprs.pr', 'smartwap', 'rcomwap', 'TATA.DOCOMO.DIVE.IN', \
                'TATA.DOCOMO.MMS', 'smartnet', 'vgprs.com']
replace_list_IN = {}

exclude_list_US = ['mms', 'isp.cingular', 'fast.metropcs.com', 'pcweb.tmobile.com', 'good.call', 'VZWAPP', \
                   'VZWADMIN', 'VZWIMS', 'CdmaNai', 'internetl', 'wap.tracfone', 'wholesale', 'simple', 'internet', \
                   'nxtgenphone', 'tfdata', 'PRODATA', 'admin.cs4glte.com', 'tethering.4g.ntelos.com', 'admin.4g.ntelos.com', \
                   'private.centennialwireless.com', 'truphone.com', 'cellular1wap']

replace_list_US = {'wap.cingular': 'phone', 'truphone.com': 'phone'}



for apn in apns:
    try:
        mcc = int(apn.attributes['mcc'].value)
        mnc = int(apn.attributes['mnc'].value)
        a = apn.attributes['apn'].value

        if a.strip() == '':
            continue

        if mcc not in full_apns.keys():
            full_apns[mcc] = {mnc: [a.strip()]}
        else:
            if mnc in full_apns[mcc].keys():
                full_apns[mcc][mnc].append(a.strip())
            else:
                full_apns[mcc][mnc] = [a.strip()]

    except:
        pass








makelist = [404, 405, 310, 311, 316] # us -> , 310, 311, 316]

for mcc in makelist:
    print ""
    print "[" + str(mcc) + "]"
    exclude_list = exclude_list_IN + exclude_list_US
    if mcc in IN_mcc:
        exclude_list = exclude_list_IN
    if mcc in US_mcc:
        exclude_list = exclude_list_US

    mnclist = full_apns[mcc].keys()
    mnclist.sort()
    for mnc in mnclist:

        apns = list(set(full_apns[mcc][mnc]))
        if filter:
            popvals = []
            for i in range(len(apns)):
                apns[i] = apns[i].replace("'", "").replace(' ', '')
                if apns[i] in replace_list_US.keys():
                    apns[i] = replace_list_US[apns[i]]
                for e in exclude_list:
                    if e in apns[i]:
                        popvals.append(apns[i])
                        
            popvals = list(set(popvals))
            for p in popvals:
                apns.remove(p)
        
        print str(mnc) + "=" + ','.join(apns)

