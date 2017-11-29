from __future__ import print_function
import socket
import time
import sys
import json
import functools
import os
import random
import platform

def safeunicode(obj, encoding='utf-8'):
    r"""
    Converts any given object to unicode string.
        >>> safeunicode('hello')
        u'hello'
        >>> safeunicode(2)
        u'2'
        >>> safeunicode('\xe1\x88\xb4')
        u'\u1234'
    """
    t = type(obj)
    if t is unicode:
        return obj
    elif t is str:
        return obj.decode(encoding, 'ignore')
    elif t in [int, float, bool]:
        return unicode(obj)
    elif hasattr(obj, '__unicode__') or isinstance(obj, unicode):
        try:
            return unicode(obj)
        except Exception as e:
            return u""
    else:
        return str(obj).decode(encoding, 'ignore')


def sleep_seconds(seconds=0):
    time.sleep(seconds)


def recv_timeout(the_socket,timeout=1):
    the_socket.setblocking(0)
    total_data=[]
    data=b''
    begin=time.time()
    while 1:
        #if you got some data, then break after wait sec
        if total_data and time.time()-begin>timeout:
            break
        #if you got no data at all, wait a little longer
        #elif time.time()-begin>timeout*2:
        #    break
        try:
            data=the_socket.recv(8192)
            if data:
                total_data.append(data)
                begin=time.time()
            else:
                time.sleep(0.1)
        except:
            pass
    return b''.join(total_data)


def isWindowsSystem():
    return 'Windows' in platform.system()


def isLinuxSystem():
    return 'Linux' in platform.system()

def code_from_out(out):
    out = json.loads(out.decode('GBK'))
    if out.get('error', None) or out.get('errors', None):
        return 100
    return out.get('code', 0)

def result_from_out(out):
    out = json.loads(out.decode('GBK'))
    result = out['result']
    return result

def run_command(p, cmd, params=[], seconds=0,pr=False):
    json_cmd_obj = {'jsonrpc': '2.0', 'method': cmd, 'params': params or [], 'id': 1}
    if pr:
        print(u"cmd: %s" % json.dumps(json_cmd_obj, ensure_ascii=False))
    bytesdata_origin = (json.dumps(json_cmd_obj, ensure_ascii=False) + u'\n')
    bytesdata = bytesdata_origin.encode('utf-8')
    # bytes = safeunicode(bytes)
    # if is_python3:
    #     bytesdata = bytesdata
    # else:
    #     bytesdata = safeunicode(bytesdata)
    p.send(bytesdata)
    # stdoutput = b''
    # while True:
    #     buf = p.recv(1024)
    #     if len(buf) < 1:
    #         break
    #     stdoutput += buf
    stdoutput = recv_timeout(p)
    sleep_seconds(seconds)
    return stdoutput
def run_command2(p, cmd, params={}, seconds=0,pr=False):
    json_cmd_obj = {'jsonrpc': '2.0', 'method': cmd, 'params': params or {}, 'id': 1}
    if pr:
        print(u"cmd: %s" % json.dumps(json_cmd_obj, ensure_ascii=False))
    bytesdata_origin = (json.dumps(json_cmd_obj, ensure_ascii=False) + u'\n')
    bytesdata = bytesdata_origin.encode('utf-8')
    # bytes = safeunicode(bytes)
    # if is_python3:
    #     bytesdata = bytesdata
    # else:
    #     bytesdata = safeunicode(bytesdata)
    p.send(bytesdata)
    # stdoutput = b''
    # while True:
    #     buf = p.recv(1024)
    #     if len(buf) < 1:
    #         break
    #     stdoutput += buf
    stdoutput = recv_timeout(p)
    sleep_seconds(seconds)
    return stdoutput



        
    
    











    
