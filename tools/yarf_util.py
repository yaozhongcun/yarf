

import os,re,sys
import platform

def islinux():
    a=platform.platform()
    if (1 == a.find("inux") ):
        return 1 
    else:
        return 0

def iswin():
    a=platform.platform()
    if (1 == a.find("indows") ):
        return 1 
    else:
        return 0

def first2upper(str):
    head = True
    ret = ""
    for ch in str:
        if (ch == '_'):
            head = True
            continue

        if (head):
            ret += ch.upper()
            head = False
        else:
            ret += ch
    return ret

def mkdir_wrapper(dir):
    try:
        os.mkdir(dir)
    except OSError as err:
        if (str(err).find("Errno 17] File exis") == -1):
            raise
        
