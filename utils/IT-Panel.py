#!/usr/bin/env python3
#
import os
import shutil
import sys
import subprocess
import time
import datetime
import requests
import json


def main():
    # header Http
    header = {'Content-type': 'application/json',
                "Accept": "application/json"}
    # coordonnees Emulator
    server = "192.168.1.102"
    port = 9210
    username = "admin"
    password = "@dm1n"
    # addresse a recuperer
    address = ["4001$0.0", "4001$0.1", "4001$0.3"]
    aff_valid=True
    aff_value=-1
    
    # déclaration des variable :
    #  ca peut échouer en "BadRequest si les variable sont deja declarée. dans ce cas ignorer l'erreur"
    url = f"http://{server}:{port}/variables"
    
    for a in address:
        req = {"address": a, "alias": ""}
        try:
            r=requests.post(url,
                data = json.dumps(req),
                auth = (username, password),
                timeout = 20,
                headers = header)
            if (r.status_code == 204):
                print(f"variable {a} enregistree")
            if (r.status_code == 400):
                error=json.loads(r.text)
#                print(f"variable {a} déjà enregistre :   infos:{error}")
            if (r.status_code == 401):
                error=json.loads(r.text)
                print(f"non autorise : infos:{error}")
        except Exception as e:
            # erreur de comm (socker error, bad ip, no response, timeout ...)
            print(f"Erreur de comm: {e}")

    myPopen = subprocess.Popen(['/root/rpi-rgb-led-matrix/utils/IT-Panel-1','-f','/root/rpi-rgb-led-matrix/utils/fonts/spleen-16x32.bdf','--led-gpio-mapping=adafruit-hat-pwm','--led-chain=4','--led-brightness=50','--led-slowdown-gpio=4','--led-limit-refresh=120'], stdin = subprocess.PIPE,stdout = subprocess.PIPE, stderr = subprocess.PIPE, encoding = 'ascii',bufsize=1,universal_newlines=True)

    # Get Values
    while True:
        try:
            url=f"http://{server}:{port}/variables/value?Address={address}"
            r=requests.get(url, auth = (username, password), timeout = 20, headers = header)
            if (r.status_code == 200):
                vars=json.loads(r.text)
                for v in vars:
                    if (v['address'] == "4001$0.3"):
                        if (v['value']==1):
                            aff_valid=True
                        else:
                            aff_valid=False
                    if (v['address'] == "4001$0.0"):
                        aff_value=v['value']
                if (aff_valid==True): 
                    myPopen.stdin.write(str(aff_value))
                    myPopen.stdin.write('\n')
                else :
                    myPopen.stdin.write(str(-1))
                    myPopen.stdin.write('\n')
#                    print(
#                         f"variable {v['address']} = {v['value']}. value is valid : {v['valid']}")
            
            if (r.status_code == 400):
                error=json.loads(r.text)
                print(f"variable non declarée :  infos:{error}")
            if (r.status_code == 401):
                error=json.loads(r.text)
                print(f"non autorisé : infos:{error}")
            time.sleep (1)
        except Exception as e:
            # erreur de comm (socker error, bad ip, no response, timeout ...)
            print(f"Erreur de comm: {e}")
         

#unregister variables : pour le principe, mais inutile..
'''
    for a in address:
        try:
            url = f"http://{server}:{port}/variables/{a}"
            r=requests.delete(url,                
                auth = (username, password),
                timeout = 20,
                headers = header)
            if (r.status_code == 204):
                print(f"variable {a} n'est plus enregistrée")
            if (r.status_code == 400):
                error=json.loads(r.text)
                print(f"variable {a} n'etait pas enregistrée :  infos:{error}")
            if (r.status_code == 401):
                error=json.loads(r.text)
                print(f"non autorisé : infos:{error}")
        except Exception as e:
            # erreur de comm (socker error, bad ip, no response, timeout ...)
            print(f"Erreur de comm: {e}")    
'''

main()


