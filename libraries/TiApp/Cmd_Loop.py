
from __future__ import print_function
import socket
import time
import sys
import json
import functools
import os
import random
from tdfs_thread import *
from App_core import *
from multiprocessing import Process
import subprocess
read_buf_size=4096
min_piece_size=(2<<10)

def copy_file_part(file,size,destfile):
    fd=open(destfile,'wb')
    print('Part Size:'+str(size))
    read_end=False
    while True:
        read_size=read_buf_size
        if size< read_buf_size:
            read_size=size
        data=file.read(read_size)
        fd.write(data)
        read_count=len(data)
        size-=read_count
        if read_count<1:
            break
    fd.close()

def upload_file(cmd_and_params,t):
    try:
        filename=cmd_and_params[1]
        piece=int(cmd_and_params[2])
        copy=int(cmd_and_params[3])
        price=float(cmd_and_params[4])
        desc=cmd_and_params[5]
        uploader=cmd_and_params[6]
        contract=cmd_and_params[7]
    except:
        print("param error")
    else:
        fsize = os.path.getsize(filename)
        if (fsize/piece)<min_piece_size :
            print("piece to small")
        piece_size=int((fsize+(piece-fsize%piece))/piece)
        fs=open(filename,'rb')
        filenames=[]
        filenames.append(filename)
        last_piece_index=0
        for i in range(0,piece):
            piecename=filename+'_p'+str(i)
            copy_file_part(fs,piece_size,piecename)
            filenames.append(piecename)
        #move files to share dir
        t.copy_file_to_share(filenames) 
        info=FileUploadInfo(filenames,fsize)
        info.set_addtion_info(price,copy,desc,uploader,contract)
        return info
    pass
def get_param(cmd):
    cmd = cmd.replace('\n', '')
    out = str.split(cmd, '"')
    num=len(out)
    res=[]
    for i in range (0,num):
        if i%2==1:
            res.append(out[i])
        else:
            while str.find(out[i],'  ')>=0 :
                out[i]=out[i].replace('  ',' ')
            subout=out[i].split(' ')
            for sub in subout:
                res.append(subout)






def get_param(cmd):
    cmd = cmd.replace('\n', '')
    out = str.split(cmd, '"')
    num=len(out)
    res=[]
    for i in range (0,num):
        if i%2==1:
            res.append(out[i])
        else:
            while str.find(out[i],'  ')>=0 :
                out[i]=out[i].replace('  ',' ')
            subout=out[i].split(' ')
            for sub in subout:
                if sub!='':
                    res.append(sub)
    return res

def handle_cmd(cmd,tdfs_t,run,opened_wallet):
    #cmd=cmd.replace('\n','')
    #cmd_and_params=str.split(cmd,' ')
    cmd_and_params=get_param(cmd)
    if len(cmd_and_params)==0:
        return
    print(cmd_and_params)
    if  opened_wallet ==False and cmd_and_params[0]=='open':
        tdfs_t.setpw(cmd_and_params[1])
        tdfs_t.start()
        opened_wallet=True
        return
    if opened_wallet==False and cmd_and_params[0]!='open':
        return
    if cmd_and_params[0]=='upload_file':
        ret=upload_file(cmd_and_params,tdfs_t)
        if ret.filename != '' :
            tdfs_t.wait(ret)
    elif cmd_and_params[0]=='list_upload_requests':
        tdfs_t.list_upload_requests()
    elif cmd_and_params[0]=='save_piece':
        tdfs_t.save_piece(cmd_and_params)
    elif cmd_and_params[0]=='get_file':
        tdfs_t.get_file(cmd_and_params)
    elif cmd_and_params[0]=='confirm_piece':
        tdfs_t.confirm(cmd_and_params)
    elif cmd_and_params[0]=='exit':
        tdfs_t.exit(cmd_and_params)
        run=False

def client():
    exe_name = "Ti_Value.exe"
    # if isLinuxSystem():
    #    exe_name="Ti_Value"
    tvp=subprocess.Popen(exe_name+" --rpcuser admin --rpcpassword admin --rpcport 10086 --server",stdout=subprocess.PIPE)

    out=""
    while True:
        out += tvp.stdout.read(1)
        if tvp.poll():
            print(out)
            return
        if out.find('>>>')!=-1 :
            break;
    tdfs_t=tdfs_thread()
    run=True
    opened_wallet=False;
    while run:
        cmd=sys.stdin.readline()
        handle_cmd(cmd,tdfs_t,run,opened_wallet)
    while tdfs_t.isAlive():
        pass

