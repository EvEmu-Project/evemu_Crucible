
from elementtree import ElementTree

#import cElementTree as ET
#import lxml.etree as ET
#import xml.etree.ElementTree as ET # Python 2.5
import elementtree.ElementTree as ET
from elementtree.ElementTree import XML, fromstring, tostring
import httplib2
import MySQLdb
import sys


options = [
    "invTypesBasePrice",
    "regionalMarketBuyPrice",
    "regionalMarketSellPrice",
    "regionalMarketBuySellPrices",
    "universalMarketBuyPrice",
    "universalMarketSellPrice",
    "universalMarketBuySellPrices"
    ]


#==================================================================================================
def print_usage():
    print "ERROR: Command line arguments to not match what was expected!"
    print "USAGE: > python eve-central_to_database.py <mysql_server_IP> <mysql_user> <mysql_password> <mysql_db_name> <option>"
    print "  where:"
    print "    <mysql_server_IP> : IP address of your MySQL server"
    print "    <mysql_user>      : username required of your MySQL server"
    print "    <mysql_password>  : password required for specified username for your MySQL server"
    print "    <mysql_db_name>   : name of the MySQL database you are targeting for EVE item price updates"
    print "    <option>          : options for what price data to gather and where in the database it is applied"
    print "      valid values:"
    print "        ", options[0], "- updates only 'basePrice' field of all applicable typeIDs in 'invTypes' database table"
    print "        ", options[1], "- "
    print "        ", options[2], "- "
    print "        ", options[3], "- "
    print "        ", options[4], "- "
    print "        ", options[5], "- "
    print "        ", options[6], "- "
    return;
#==================================================================================================


#==================================================================================================
def mysql_update_invTypes_basePrice(mysql_db, mysql_cursor, typeID, avg_sell_price, verbose):
    # Create mysql query to update basePrice of supplied typeID with the supplied avg_sell_price:
    
    # NOTE: basePrice field is 1000 times actual price!
    
    query_string = "UPDATE `invTypes` SET `basePrice` = " + str(avg_sell_price) + " WHERE `typeID` = " + str(typeID)

    if (verbose == 1):
        print "QUERY = \"", query_string, "\""

    try:
        mysql_cursor.execute(query_string)

        results = mysql_cursor.fetchall()
    
        if (verbose == 1):
            print "RESP = \"", results, "\""

        mysql_db.commit()
    except:
        mysql_db.rollback()
        
    return results
#==================================================================================================


#==================================================================================================
def eve_central_query_market_data(regionID,option):

    if (option == options[0]):
        # HTTP GET from EVE-Central website using URL API for a typeID to get market data XML response:
        # See API here:  http://dev.eve-central.com/evec-api/start
        url_str = "http://api.eve-central.com/api/marketstat?typeid=" + str(typeID)
        resp, contents = httplib2.Http().request(url_str)
        return contents

    if (option == options[1]):
        print "NOT SUPPORTED YET!"

    if (option == options[2]):
        print "NOT SUPPORTED YET!"

    if (option == options[3]):
        print "NOT SUPPORTED YET!"

    if (option == options[4]):
        print "NOT SUPPORTED YET!"

    if (option == options[5]):
        print "NOT SUPPORTED YET!"

    if (option == options[6]):
        print "NOT SUPPORTED YET!"

    return ""
#==================================================================================================
        

#==================================================================================================
#==================================================================================================
#       BEGIN CORE PROGRAM
#==================================================================================================
#==================================================================================================

# Check command line arguments:
if (len(sys.argv) < 6):
    print_usage()
    sys.exit(0)

# Capture command line options into local variables:
mysql_server_IP = sys.argv[1]
mysql_user = sys.argv[2]
mysql_password = sys.argv[3]
mysql_db_name = sys.argv[4]
option = sys.argv[5]

# Enable 'verbose' mode if hidden parameter is specified:
if (len(sys.argv) > 6):
    if (sys.argv[6] == "verbose"):
        verbose = 1
    else:
        verbose = 0
else:
    verbose = 0

# Check valid <option> values:
option_valid = 0
if (option == options[0]):
    option_valid = 1
if (option == options[1]):
    option_valid = 1
if (option == options[2]):
    option_valid = 1
if (option == options[3]):
    option_valid = 1
if (option == options[4]):
    option_valid = 1
if (option == options[5]):
    option_valid = 1
if (option == options[6]):
    option_valid = 1
if (option_valid == 0):
    print_usage()
    sys.exit(0)

# Unit Test command line argument extraction:
print ""
print "Arguments you entered:"
print "<mysql_server_IP> = ", mysql_server_IP
print "<mysql_user> = ", mysql_user
print "<mysql_password> = ", mysql_password
print "<mysql_db_name> = ", mysql_db_name
print "option = ", option
print ""
#sys.exit(0)
    
# MySQLDB: Connect to the MySQL Database
db = MySQLdb.connect(
        host = mysql_server_IP,
        user = mysql_user,
        passwd = mysql_password,
        db = mysql_db_name )

print "CONNECTED TO DATABASE '", mysql_db_name, "' as user '", mysql_user, "'"
print ""

# MySQLDB: prepare a cursor object using cursor() method
cursor = db.cursor()

# MySQLDB: Query mapRegions database table to get list of regionIDs
print "Getting mapRegions..."
cursor.execute("\
 SELECT\
   `regionID`, `regionName`\
 FROM `mapRegions`\
 WHERE 1")
print "DONE!"
print ""

results = cursor.fetchall()

regionID_List = []
regionName_List = []

for row in results:
    regionID_List.append(row[0])
    regionName_List.append(row[1])

# Unit test region information collection:
#print regionID_List
#print len(regionID_List)
#print regionName_List
#print len(regionName_List)

regionProgress = 0

if (option == options[0]):
    regionID_List_length = 1
else:
    regionID_List_length = len(regionID_List)


# MySQLDB: Query invTypes database table to get list of typeIDs
print "Getting invTypes..."
cursor.execute("\
 SELECT\
   `typeID`, `typeName`, `basePrice`\
 FROM `invTypes`\
 WHERE\
   `groupID` IN\
     (SELECT `groupID` FROM `invGroups` WHERE `categoryID` IN\
       (4,5,6,7,8,9,16,17,18,20,22,23,24,25,32,34,35,39,40,41,42,43,46))")
print "DONE!"
print ""

results = cursor.fetchall()

typeID_List = []
typeName_List = []

for row in results:
    typeID_List.append(row[0])
    typeName_List.append(row[1])

# Unit test typeID information collection:
#print typeID_List
#print len(typeID_List)
#print typeName_List
#print len(typeName_List)

typeID_Progress = 0
typeID_List_length = len(typeID_List)


# Outer Loop - Loop through all regionIDs
print "PROCESSING REQUEST..."
for region in range(0,regionID_List_length):
    regionID = regionID_List[region]
    regionProgress = 100.0 * ((1.0*region)/regionID_List_length)

    # Inner Loop - Loop through entire list of typeIDs
    for index in range(0,range(1,typeID_List_length-1)):
        typeID = typeID_List[index]
        typeID_Progress = 100.0 * ((1.0*index)/typeID_List_length)

        if (verbose == 1):
            print "Getting eve-central data for typeID ", typeID_List[index], " - ", typeName_List[index]
        contents = eve_central_query_market_data(regionID,option)
        if (verbose == 1):
            print "Response from eve-central:"
            print contents
            print "END RESPONSE"
        
        if (contents[0:5] == "<?xml"):
            # Parse the XML response from EVE-Central
            # See usage here for ElementTree:  http://effbot.org/zone/element-index.htm
            tree = XML(contents)        # From a string
            if (verbose == 1):
                print "XML received from eve-central:"
                print tree
            #print "XML Element count = ", len(tree)

            # the tree root is the toplevel html element
            tree_map = dict((c, p) for p in tree.getiterator() for c in p)
            root = tree

            for c in root.getchildren():
                if (verbose == 1):
                    print c.tag
                for d in c.getchildren():
                    if (verbose == 1):
                        print "    ", d.tag
                    for e in d.getchildren():
                        if (verbose == 1):
                            print "        ", e.tag
                        for f in e.getchildren():
                            if (verbose == 1):
                                print "            ", f.tag, " = ", f.text
                            if ((e.tag == "sell") and (f.tag == "avg")):
                                avg_sell_price = float(f.text)
                            if ((e.tag == "buy") and (f.tag == "avg")):
                                avg_buy_price = float(f.text)

                            # Perform Specified Action on MySQL Database now that we have the Price data:
                            if ((e.tag == "sell") and (f.tag == "avg")):
                                if (option == options[0]):
                                    #create mysql query to update table data
                                    mysql_update_invTypes_basePrice(db, cursor, typeID, avg_sell_price, verbose)
                                    print "[%.2f" % typeID_Progress, "%] ", "typeID", int(typeID), "[", typeName_List[index], "]  BASE PRICE UPDATED: ", avg_sell_price, "ISK"
                                
                                if (option == options[1]):
                                    print "OPTION '", options[1], "' not supported at this time!"
                                
                                if (option == options[2]):
                                    print "OPTION '", options[2], "' not supported at this time!"
                                
                                if (option == options[3]):
                                    print "OPTION '", options[3], "' not supported at this time!"
                                
                                if (option == options[4]):
                                    print "OPTION '", options[4], "' not supported at this time!"
                                
                                if (option == options[5]):
                                    print "OPTION '", options[5], "' not supported at this time!"
                                
                                if (option == options[6]):
                                    print "OPTION '", options[6], "' not supported at this time!"
                            
                    #print "typeID=", int(d.attrib.get("id")), ": AVG_SELL =", avg_sell_price, " AVG_BUY =", avg_buy_price
        else:
            print "[%.2f" % typeID_Progress, "%] ", "typeID", int(typeID), "[", typeName_List[index], "]  NO DATA... SKIPPING..."

print "DONE!"

# if you need the root element, use getroot
#root = tree.getroot()

# ...manipulate tree...

#tree.write("out.xml")

# MYSQLDB: Close the connection to the database so we can exit
print "CLOSING DATABSE CONNECTION"
db.close()

sys.exit(0)

#==================================================================================================
#==================================================================================================
#       END CORE PROGRAM
#==================================================================================================
#==================================================================================================

