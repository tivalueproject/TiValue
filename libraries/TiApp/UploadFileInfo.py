from __future__ import print_function
import socket
import time
import sys
import json
import functools
import os
import random
from App_core import *
class rpc_request:
    def __init__(self,method,param):
        self.method=method
        self.param=param

    def send_and_recv(self,p):
        return run_command(p,self.method,self.param,0,True)

class FileUploadInfo:
    def __init__(self,names,size):
        piece_num=len(names)
        self.filename=names[0]
        self.file_hash=''
        self.pieces=list()
        self.pieces_id_got=list()
        self.pieces_size=list()
        self.size=size
        self.price=0
        self.copy=0
        self.desc=''
        self.uploader=''
        self.contract=''
        for i in range(1,piece_num):
            self.pieces.append(names[i])
            self.pieces_id_got.append('')
            self.pieces_size.append(0)
            
    def fix_dir(self,newdir):
        self.filename=newdir+'\\'+os.path.basename(self.filename)
        for i in range(0,len(self.pieces)):
            self.pieces[i]=newdir+'\\'+os.path.basename(self.pieces[i])

    def set_addtion_info(self,price,copy,desc,uploader,contract):
        self.price=price
        self.copy=copy
        self.desc=desc
        self.uploader=uploader
        self.contract=contract


    def set_file_info(self,full_path,file_hash,size):
        if os.path.abspath(self.filename)==os.path.abspath(full_path):
            self.file_hash=file_hash
            self.size=size
            return True
        for i in range(0,len(self.pieces)):
            if os.path.abspath(self.pieces[i])==os.path.abspath(full_path):
                self.pieces_id_got[i]=file_hash
                self.pieces_size[i]=size
                
                return True
        return False

    def is_finish(self):
        if self.file_hash =='':
            return False
        for piece_id in self.pieces_id_got:
            if piece_id=='':
                return False
        return True

    def get_upload_params(self):
        res=list()
        res.append(self.uploader)
        res.append(self.contract)
        res.append(os.path.basename(self.filename))
        res.append(self.size)
        res.append(self.desc)
        pieces_info=self.file_hash+';'
        for i in range(0,len(self.pieces)):
            pieces_info+=self.pieces_id_got[i]+','+str(self.pieces_size[i])+';'
        res.append(pieces_info)
        res.append('TV')
        res.append(self.price)
        res.append(self.copy)
        res.append(len(self.pieces))
        res.append(3)
        res.append('')
        res.append(10)
        return res