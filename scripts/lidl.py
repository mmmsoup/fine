#!/bin/python3

import json
import os
import sys

# https://github.com/Andre0512/lidl-plus
from lidlplus import LidlPlusApi

step = 1;
def printstep(message):
    global step
    print("\033[1mstep "+str(step)+"\033[0m - "+message)
    step += 1

configfile = open("lidlconfig.json")
config = json.load(configfile)
configfile.close()

if sys.argv[1] == "dl":
    if len(sys.argv) != 3:
        exit(1)
    monthstr = sys.argv[2]
    jsonfilename = config["jsondir"]+"/"+monthstr+".json"

    try:
        os.makedirs(config["jsondir"])
    except:
        pass

    printstep("connecting to Lidl server and getting list of receipts")
    lidl = LidlPlusApi(config["language"], config["region"], refresh_token=config["refresh_token"])
    receipts = lidl.tickets()

    printstep("downloading JSON data for tickets from '"+monthstr+"' and extracting useful data to '"+jsonfilename+"'")
    transactions = []
    for i in range(len(receipts)):
        if receipts[i]["date"][0:7] == monthstr:
            receipt = lidl.ticket(receipts[i]["id"])
            transactions.append({
                "date": receipt["date"][0:10],
                "items": list(map(lambda item: {"name": item["name"], "price": item["originalAmount"], "isweight": item["isWeight"], "quantity": item["isWeight"] and float(item["quantity"]) or int(item["quantity"]) }, receipt["itemsLine"])),
                "amount": receipt["payments"][0]["amount"],
                "card_acct_no": receipt["payments"][0]["type"] == "CreditCard" and receipt["payments"][0]["cardInfo"]["accountNumber"] or "cash"
            })
            print("\r"+str(len(transactions))+" tickets extracted", end="")

    jsonfile = open(jsonfilename, "w")
    jsonfile.write(json.dumps(transactions))
    jsonfile.close()
elif sys.argv[1] == "mkadj":
    if len(sys.argv) != 3:
        exit(1)
    monthstr = sys.argv[2]

    printstep("making adjustments for transactions from '"+monthstr+"'")

    jsonfilename = config["jsondir"]+"/"+monthstr+".json"
    jsonfile = open(jsonfilename, "r")
    jsondata = json.load(jsonfile)
    jsondata.reverse()
    jsonfile.close()
    try:
        os.makedirs(config["csvdir"])
    except:
        pass
    csvfilename = config["csvdir"]+"/"+monthstr+".csv"
    csvfile = open(csvfilename, "w")
    csvfile.write("Lidl adjustments for "+monthstr+"\n")
    adjustmentsnum = 0
    for transaction in jsondata:
        for i in range(len(transaction["items"])):
            name = transaction["items"][i]["name"]
            if name in config["rules"]:
                csvfile.write(transaction["date"]+","+transaction["items"][i]["price"]+",\""+name+"\","+config["default_category"]+"\n")
                csvfile.write(transaction["date"]+",-"+transaction["items"][i]["price"]+",\""+name+"\","+config["rules"][name]+"\n")
                adjustmentsnum += 1
                print("\r"+str(adjustmentsnum)+" adjustments made", end="")
    csvfile.close()
elif sys.argv[1] == "items":
    itemslist = []
    attributesdict = {}
    for filename in os.listdir(config["jsondir"]):
        jsonfile = open(config["jsondir"]+"/"+filename, "r")
        jsondata = json.load(jsonfile)
        jsonfile.close()
        for transaction in jsondata:
            for item in transaction["items"]:
                if item["name"] not in itemslist:
                    itemslist.append(item["name"])
                    attributesdict[item["name"]] = { "isweight": item["isweight"], "quantity": item["quantity"] }
                else:
                    attributesdict[item["name"]]["quantity"] += item["quantity"]
    itemslist.sort()
    for item in itemslist:
        if item in config["rules"]:
            print("\033[1m"+str(attributesdict[item]["quantity"])+(attributesdict[item]["isweight"] and "kg\t" or "\t")+item+(" " * (21-len(item)))+"("+config["rules"][item]+")\033[0m")
        else:
            print(str(attributesdict[item]["quantity"])+(attributesdict[item]["isweight"] and "kg\t" or "\t")+item)
elif sys.argv[1] == None:
    print("missing required subcommand")
else:
    print("unknown subcommand '"+sys.argv[1]+"'")
