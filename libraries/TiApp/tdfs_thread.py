from __future__ import print_function
import socket
import time
import sys
import json
import functools
import os
import random
import shutil
import threading
from App_core import *
from UploadFileInfo import *
tdfs_port=12345
chain_rpc_payload = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"login\",\"params\":[\"admin\",\"admin\"]}"



class tdfs_thread(threading.Thread):
    def __init__(self):  
        threading.Thread.__init__(self)
        self.lock=threading.Lock()
        self.td_lock=threading.Lock()
        self.ch_lock=threading.Lock()
        self.tdfs_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.tdfs_sock.connect(('127.0.0.1', tdfs_port))
        self.filegroup=list()
        self.thread_stop=False
        self.chain_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.chain_sock.connect(('127.0.0.1', 10086))
        self.chain_sock.send(chain_rpc_payload.encode('utf-8'))
        self.chain_sock.recv(10240)
        self.tdr_lock=threading.Lock()
        self.chr_lock=threading.Lock()
        self.chain_requests=[]
        self.tdfs_requests=[]
        self.downloading_files=[]
        self.shareing_files=[]
        self.td_lock.acquire()
        out=run_command(self.tdfs_sock,"GetIncomingDir",[])
        self.download_dir=result_from_out(out)['IncomingDir']
        out=run_command(self.tdfs_sock,"GetSharedDirs",[])
        self.share_dirs=result_from_out(out)
        self.node_id='0'
        self.pw=""
        while int(self.node_id,16)==0:
            out=run_command(self.tdfs_sock,"GetNodeKadId",[])
            self.node_id=result_from_out(out)['NodeKadID']
        self.td_lock.release()
        self.ch_lock.acquire()
        self.downloaded_files=[]
        out = run_command(self.chain_sock, "wallet_list", [])
        out=result_from_out(out)
        if 'op' in out:
            out=run_command(self.chain_sock,"open",['op'])
            out=run_command(self.chain_sock,"unlock",[123456789,self.pw])
        else:
            out = run_command(self.chain_sock, "create", ['op',self.pw])
        print(out)
        out=run_command(self.chain_sock,"wallet_set_node_id",[self.node_id])
        self.ch_lock.release()
        if out:
            print(out)
    def setpw(self,pw):
        self.pw=pw
    def send_upload_request(self,grp):
        self.ch_lock.acquire()
        out=run_command(self.chain_sock,'store_file_to_network',grp.get_upload_params())
        self.ch_lock.release()
        if code_from_out(out) == 0:
            return True
        print(out)
        return False

    def need_no_new_download(self, fname, hash):
        for f in self.downloading_files:
            if  f['FileHash']==hash:
                return True
        for f in self.shareing_files:
            if  f['FileHash']==hash:
                return True
        return False
    def is_downloaded(self,fname,hash):
        for f in self.shareing_files:
            if f['FileHash']==hash:
                print(hash+" "+f['FilePath']+'\\'+f['FileName'])
                return f['FilePath']+'\\'+f['FileName']
        return ''
    def is_downloading(self,fname,hash):
        for f in self.downloading_files:
            if f['FileName']==fname and  f['FileHash']==hash:
                return True
        return False
    def assemble_file(self,filename,piece_name=[]):
        print(piece_name)
        out=open(self.share_dirs[0]+'\\'+ filename,'wb')
        for infpath in piece_name:
            inf=open(infpath,'rb')
            while True:
                data=inf.read(4096)
                if len(data)<=0:
                    break
                out.write(data)
            inf.close()
        out.close()

    def download(self,fid):
        self.ch_lock.acquire()
        out=run_command(self.chain_sock,"blockchain_get_file_save_node",[fid])
        self.ch_lock.release()
        if code_from_out(out)!=0:
            return
        res=result_from_out(out)
        i=0
        filename=res['filename']
        print(res)
        self.ch_lock.acquire()
        out=run_command(self.chain_sock,"generate_download_validation",[fid])
        self.ch_lock.release()
        if code_from_out(out)!=0:
            return
        validate_info=result_from_out(out)
        c_id=res['c_id']
        if self.is_downloaded('',c_id)!='':
            print("ffex")
            return
        print("und c="+str(len(res['pieces'])))

        downloaded_piece_names = []
        undownloaded_piece_count=len(res['pieces'])
        for piece_info in res['pieces']:
            piece_id=piece_info['piece_id']
            piece_name=filename+'_p'+str(i)
            i+=1
            print(piece_id)
            newfn=self.is_downloaded(piece_name, piece_id)
            if newfn!='':
                undownloaded_piece_count-=1
                downloaded_piece_names.append(newfn)
            else:
                if self.is_downloading(piece_name, piece_id):
                    print('is downloading')
                    continue
                if len(piece_info['nodes'])<1:
                    print('nodes 0')
                    return
                node_id=piece_info['nodes'][0]
                self.td_lock.acquire()
                param=dict()
                param['FileHash']=piece_id
                param['FileName']=piece_name
                param['FileSize'] = piece_info['size']
                param['AuthInfo'] = validate_info
                param['SrcHash'] = piece_info['nodes'][0]
                out=run_command(self.tdfs_sock,"AddDownloadFile",param,pr=1)
                print(out)
                self.td_lock.release()
            if undownloaded_piece_count==0:
                self.assemble_file(filename,downloaded_piece_names)





    def copy_file_to_share(self,filenames):
        for i in range(0,len(filenames)):
            fn=filenames[i]
            shutil.copyfile(fn,self.share_dirs[0]+'\\'+os.path.basename(fn))
            print(fn+'   '+self.share_dirs[0]+'\\'+os.path.basename(fn))
            self.td_lock.acquire()
            run_command(self.tdfs_sock,'ReloadSharedFiles',[])
            self.td_lock.release()

    def run(self): #Overwrite run() method, put what you want the thread do here  
        while not self.thread_stop:
            self.ch_lock.acquire()
            out=run_command(self.chain_sock,"rescan",[])
            self.ch_lock.release()
            time.sleep(10)
            self.lock.acquire()
            self.td_lock.acquire()
            out=run_command(self.tdfs_sock,"GetSharedFiles",[])
            self.td_lock.release()
            out = json.loads(out.decode('GBK'))
            res=out['result']
            if len(res)>0:
                self.shareing_files=res
            self.lock.release()
            out=run_command(self.tdfs_sock,"GetDownloadFiles",[])
            self.downloading_files=[]

            self.downloading_files=result_from_out(out)
            for fi in self.downloading_files:
                if fi['Status']==9:
                    self.downloading_files.remove(fi)
                    fi['FilePath']=self.share_dirs[0]
                    self.shareing_files.append(fi)
                    shutil.copyfile(self.download_dir+'\\'+fi['FileName'],self.share_dirs[0]+'\\'+fi['FileName'])
                    param=dict();
                    param['FileHash']=fi['FileHash']
                    run_command(self.tdfs_sock,'ClearCompleted',param)
                    run_command(self.tdfs_sock,'ReloadSharedFiles',[])
            self.td_lock.acquire()
            out=run_command(self.tdfs_sock,"GetSharedFiles",[])
            self.td_lock.release()
            self.lock.acquire()
            out = json.loads(out.decode('GBK'))
            ress=out['result']
            for res in ress:
                fullpath=res['FilePath']+'\\'+res['FileName']
                file_id=res['FileHash']
                file_size=res['FileSize']
                remove=False
                for grp in self.filegroup:
                    if grp.set_file_info(fullpath,file_id,file_size):
                        if grp.is_finish() :
                            self.send_upload_request(grp)
                            remove=True
                        break
                if remove:
                    self.filegroup.remove(grp)
            self.lock.release()
            self.chr_lock.acquire()
            print(self.chain_requests)
            for req in self.chain_requests:
                self.ch_lock.acquire()
                out=req.send_and_recv(self.chain_sock)
                if code_from_out(out)==0:
                    self.chain_requests.remove(req)
                self.ch_lock.release()
            self.chr_lock.release()
            self.ch_lock.acquire()
            out=run_command(self.chain_sock,"wallet_get_my_store_request",[])
            self.ch_lock.release()
            if code_from_out(out)==0:
                res=result_from_out(out)
                print(res)
                for entry in res:
                    fna=entry['filename']+'_p'+str(entry['piece_index'])
                    if self.need_no_new_download(fna, entry['c_id']) ==False and self.need_no_new_download(fna, entry['piece_id']) ==False:
                        self.ch_lock.acquire()
                        out = run_command(self.chain_sock, "generate_download_validation",
                                          [entry['file_id']['file_id'] + entry['file_id']['uploader']], pr=True)
                        self.ch_lock.release()
                        print(out)
                        if code_from_out(out) != 0:
                            continue
                        validate_info = result_from_out(out)
                        self.td_lock.acquire()
                        print("AddDownloadFile")
                        param=dict()
                        param['FileHash']=entry['piece_id']
                        param['FileName']=fna
                        param['FileSize']=entry['piece_size']
                        param['SrcHash']=entry['node_id']
                        param['AuthInfo']=validate_info
                        print(param)
                        out=run_command(self.tdfs_sock,"AddDownloadFile",param,pr=True)
                        self.td_lock.release()
            self.ch_lock.acquire()
            out=run_command(self.chain_sock,"wallet_get_my_access",[])
            self.ch_lock.release()
            if code_from_out(out)!=0:
                continue
            res=result_from_out(out)
            for access_info in res:
                for fid in access_info['file_id']:
                    self.download(fid)
        run_command(self.chain_sock,"close",[])





            
    def list_upload_requests(self):
        self.ch_lock.acquire()
        out=run_command(self.chain_sock,"blockchain_get__upload_requests",[])
        self.ch_lock.release()
        print(out)
        if code_from_out(out) == 0:
            return

    def stop(self):  
        self.thread_stop = True  
    def wait(self,info):
        self.lock.acquire()
        info.fix_dir(self.share_dirs[0])
        self.filegroup.append(info)
        self.lock.release()

    def save_piece(self,cmd_and_params):
        requester=cmd_and_params[1]
        file_id=cmd_and_params[2]
        file_piece_id=cmd_and_params[3]
        node_id=''
        exec_limit=10
        method="store_file_piece"
        self.chr_lock.acquire()
        self.chain_requests.append(rpc_request(method,[requester,file_id,file_piece_id,node_id,exec_limit]))
        self.chr_lock.release()
        
    def get_file(self,cmd_and_params):
        requester=cmd_and_params[1]
        file_id=cmd_and_params[2]
        exec_limit=10
        method="get_file_access"
        self.chr_lock.acquire()
        self.chain_requests.append(rpc_request(method,[requester,file_id,exec_limit]))
        self.chr_lock.release()

    def confirm(self,cmd_and_params):
        confirmer=cmd_and_params[1]
        file_id=cmd_and_params[2]
        file_piece_id=cmd_and_params[3]
        storer_key=cmd_and_params[4]
        exec_limit=10
        self.ch_lock.acquire()
        out=run_command(self.chain_sock,'confirm_piece_saved',[confirmer,file_id,file_piece_id,storer_key,exec_limit])
        self.ch_lock.release()
        if code_from_out(out)!=0:
            print(out)


    def exit(self,cmd_and_params):
       self.thread_stop=True