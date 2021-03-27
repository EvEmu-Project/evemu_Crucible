# AlasiyaEVE basePrice updater.
# - Groove
import json
import MySQLdb
import urllib2
import sys
import time

def get_inv_types(cursor):
    cursor.execute("SELECT typeID, typeName FROM invTypes WHERE published = 1 AND marketGroupID != 0 AND groupID IN (SELECT groupID FROM invGroups Where categoryID IN (4, 5, 6, 7, 8, 9, 16, 17, 18, 20, 22, 23, 24, 25, 32, 34, 35, 39, 40, 41, 42, 43, 46))")

    results = cursor.fetchall()
    invTypes = {}
    for row in results:
        invTypes[row[0]] = row[1]

    return invTypes

def get_pricing(invTypes):
    pricing = {}
    type_count = len(invTypes)
    type_skip = 250
    ic = (type_count / type_skip)
    ids = invTypes.keys()
    for i in range(ic):
        if len(ids) < type_skip:
            type_skip = len(ids)
        types = ",".join(map(str, ids[:type_skip]))
        ids = ids[type_skip:]
        url = "https://market.fuzzwork.co.uk/aggregates/?station=60003760&types=" + types

        print " > Making request " + str(i) + " of " + str(ic)
        res = urllib2.urlopen(url)
        data = res.read()
        time.sleep(0.3)

        p = json.loads(data)
        pricing.update(p)

    return pricing

def main():
    if (len(sys.argv) < 5):
        print "usage: " + sys.argv[0] + " <host> <username> <password> <database>"
        sys.exit(0)

    # Capture command line options into local variables:
    mysql_host = sys.argv[1]
    mysql_user = sys.argv[2]
    mysql_password = sys.argv[3]
    mysql_database = sys.argv[4]

    db = MySQLdb.connect(host = mysql_host, user = mysql_user,
            passwd = mysql_password, db = mysql_database)

    cursor = db.cursor()

    print " > Collecting invTypes from database"
    inv_types = get_inv_types(cursor)

    print " > Attempting to get pricing..."
    pricing = get_pricing(inv_types)

    for typeID in inv_types.keys():
        try:
            sell_price = pricing[str(typeID)]["sell"]["weightedAverage"]
            if sell_price == 0:
                sell_price = pricing[str(typeID)]["buy"]["weightedAverage"]
                if sell_price == 0:
                    print " >> Skipping typeID: " + str(typeID) + " no weightedAverage"
                    continue

            try:
                #cursor.execute("UPDATE invTypes SET basePrice = " + str(sell_price) + " WHERE typeID = " + str(typeID))
                print " UPDATE invTypes SET basePrice = " + str(sell_price) + " WHERE typeID = " + str(typeID) + ";"
            except Exception as e:
                print "-- >> Error updating base price of " + inv_types[typeID]

            #print " > " + inv_types[typeID] + " -> " + str(sell_price)




        except KeyError:
            print "-- >> Skipping typeID: " + str(typeID) + " market data missing"


if __name__ == "__main__":
    main()
