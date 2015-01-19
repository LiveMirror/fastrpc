#!/usr/bin/python
#-*- coding:utf-8 -*-

import socket, logging
import select, errno
import os
import sys
import traceback
import gzip
from StringIO import StringIO
import Queue
import threading
import time
import thread
import cgi
from cgi import parse_qs
import json
import imp
from os.path import join, getsize
import re

##################user config ##################
logger = logging.getLogger("network-server")
static_file_dir = "static"
MaxReadSize = 1024*1024*1024
#############################################

static_dir = "/%s/" % static_file_dir
read_cache_dir = "read_cache"
cache_static_dir = "cache_%s" % static_file_dir
if not os.path.exists(cache_static_dir):
    os.makedirs(cache_static_dir)
if not os.path.exists(read_cache_dir):
    os.makedirs(read_cache_dir)
gzipdic = {"HTM":"text/html",
        "HTML":"text/html",
        "CSS":"text/css",
        "JS":"text/javascript",
        "TXT":"text/plain",
        "XML":"text/xml"}

mimedic = {"HTM":"text/html",
        "HTML":"text/html",
        "CSS":"text/css",
        "JS":"text/javascript",
        "TXT":"text/plain",
        "XML":"text/xml",
        "APK":"application/vnd.android.package-archive",
        "PXL":"application/iphone",
        "IPA":"application/iphone"}

def getTraceStackMsg():
    tb = sys.exc_info()[2]
    msg = ''
    for i in traceback.format_tb(tb):
        msg += i
    return msg

#action_dic = {}
action_time = {}
listfile = os.listdir("./")
for l in listfile:
    if l == str(__file__):
        continue
    prefixname, extname = os.path.splitext(l)
    if extname == ".py":
        try:
            __import__(prefixname)
        except Exception, e:
            print str(e)+getTraceStackMsg()
            continue
        mtime = os.path.getmtime(l)
        action_time[prefixname] = mtime
        #action_dic[prefixname] = action

class FeimatLog():
    def __init__(self, filename):
        logtime = time.strftime(".%Y-%m-%d")
        self.curlogname = filename + logtime
        self.f = open(self.curlogname, "a")
        self.basename = filename

    def __del__(self):
        self.f.close()

    def log(self, msg):
        curlogtime = time.strftime(".%Y-%m-%d")
        detaillogtime = time.strftime("[%Y-%m-%d %H:%M:%S] ")
        curlogname = self.basename + curlogtime
        if curlogname != self.curlogname:
            self.f.close()
            self.curlogname = curlogname
            self.f = open(self.curlogname, "a")
        self.f.write(detaillogtime + msg+"\n")
        self.f.flush()
        pass

class QuickHTTPRequest():
    def parse(self, param):
        self.client_ip = param["addr"][0]
        self.client_port = param["addr"][1]
        if "rc" in param:
            fp = param["rc"]
            data = ""
            while True:
                subdata = fp.read(1024)
                if subdata == "":
                    break
                data += subdata
                headend = data.find("\r\n\r\n")
                if headend > 0:
                    break
            fp.seek(0)
        else:
            data = param["toprocess"]
            headend = data.find("\r\n\r\n")
            fp = StringIO(data)
        if headend > 0:
            headlist = data[0:headend].split("\r\n")
        else:
            headlist = data.split("\r\n")
        first_line = headlist.pop(0)
        self.command, self.path, self.http_version, =  re.split('\s+', first_line)
        indexlist = self.path.split('?')
        self.baseuri = indexlist[0]
        indexlist = self.baseuri.split('/')
        while len(indexlist) != 0:
            self.index = indexlist.pop()
            if self.index == "":
                continue
            else:
                self.action,self.method = os.path.splitext(self.index)
                self.method = self.method.replace('.', '')
                break
        self.headers = {}
        for item in headlist:
            if item.strip() == "":
                continue
            segindex = item.find(":")
            if segindex < 0:
                continue
            key = item[0:segindex].strip()
            value = item[segindex+1:].strip()
            self.headers[key.lower()] = value

        self.command = self.command.lower()
        self.getdic = {}
        self.form = {}
        self.filedic = {}
        if self.command  == "get" and "?" in self.path:
            self.getdic = parse_qs(self.path.split("?").pop())
            for key in self.getdic:
                self.getdic[key] = self.getdic[key][0]
        elif self.command == "post" and self.headers.get('content-type',"").find("boundary") > 0:
            cgiform = cgi.FieldStorage(fp=fp,headers=None,
                    environ={'REQUEST_METHOD':self.command,'CONTENT_TYPE':self.headers['content-type'],})
            for key in cgiform:
                fileitem = cgiform[key]
                if fileitem.filename == None:
                    self.form[key] = fileitem.file.read()
                else:
                    self.filedic[key] = fileitem
        elif self.command == "post":
            self.form = parse_qs(data[headend+4:])
            for key in self.form:
                self.form[key] = self.form[key][0]
        if "rc" in param:
            param["rc"].close()
            del param["rc"]

def sendfilejob(aclog, request, data, epoll_fd, fd):
    try:
        base_filename = request.baseuri[request.baseuri.find(static_dir)+1:]
        cache_filename = "./cache_"+base_filename
        filename = "./"+base_filename
        return_content_type = None
        if not os.path.exists(filename) or ".." in base_filename:
            res = "404 Not Found"
            data["writedata"] = "HTTP/1.1 404 Not Found\r\nContent-Length: %s\r\nConnection:keep-alive\r\n\r\n%s" % (len(res),res)
            aclog.log(" fail: %s not found" % (request.path))
        else:
            lasttimestr = request.headers.get("if-modified-since", None)
            filemd5 = os.path.getmtime(filename)
            ctime = os.path.getctime(filename)
            timestr = time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime(filemd5))
            curtime = time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime(time.time()))
            sock = data["connections"]
            if lasttimestr == timestr and "range" not in request.headers:
                data["writedata"] = "HTTP/1.1 304 Not Modified\r\nLast-Modified: %s\r\nETag: \"%s\"\r\nDate: %s\r\nConnection:keep-alive\r\n\r\n" % (timestr,filemd5,curtime)
            else:
                ext = request.method
                iszip = False
                Accept_Encoding = request.headers.get("accept-encoding", "")
                ext_upper = ext.upper()
                return_content_type = mimedic.get(ext_upper, None)
                if ext_upper in gzipdic or \
                        (ext == "" and "gzip" in Accept_Encoding and os.path.getsize(filename) < 1024*1024*5):
                    need_zip = False
                    if not os.path.exists(cache_filename):
                        need_zip = True
                    else:
                        cache_mtime = os.path.getmtime(cache_filename)
                        if cache_mtime < ctime or cache_mtime < filemd5:
                            need_zip = True
                    if need_zip:
                        d,f = os.path.split(cache_filename)
                        try:
                            if not os.path.exists(d):
                                os.makedirs(d)
                            f_out = gzip.open(cache_filename, 'wb')
                            f_out.write(open(filename).read())
                            f_out.close()
                        except Exception, e:
                            print str(e)
                            pass
                    filename = cache_filename
                    iszip = True


                filesize = os.path.getsize(filename)
                if "range" in request.headers:
                    range_value = request.headers["range"].strip(' \r\n')
                    range_value = range_value.replace("bytes=", "")
                    start,end = range_value.split('-')
                    if end == '':
                        end = filesize - 1
                    start = int(start)
                    end = int(end)
                    headstr = "HTTP/1.1 206 Partial Content\r\nLast-Modified: %s\r\nETag: \"%s\"\r\nDate: %s\r\n" % (timestr,filemd5,curtime)
                    headstr += "Accept-Ranges: bytes\r\nContent-Range: bytes %s-%s/%s\r\n" % (start,end,filesize)
                else:
                    start = 0
                    end = filesize - 1
                    headstr = "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nLast-Modified: %s\r\nETag: \"%s\"\r\nDate: %s\r\n" % (timestr,filemd5,curtime)
                    if return_content_type != None:
                        headstr += "Content-Type: %s\r\n" % return_content_type
                offset = start
                totalsenlen = end - start + 1
                if totalsenlen < 0:
                    totalsenlen = 0
                if iszip:
                    headstr += "Content-Encoding: gzip\r\n"
                headstr += "Content-Length: %s\r\nConnection:keep-alive\r\n" % totalsenlen
                headstr += "\r\n"
                f = open(filename)
                f.seek(offset)
                readlen = 102400
                if readlen > totalsenlen:
                    readlen = totalsenlen
                firstdata = f.read(readlen)
                headstr += firstdata
                totalsenlen -= len(firstdata)
                data["f"] = f
                data["totalsenlen"] = totalsenlen
                data["writedata"] = headstr
            aclog.log(" success: %s" % (request.path))
    except Exception, e:
        aclog.log(" fail: %s %s" % (request.path, str(e)+getTraceStackMsg()))
        res = "404 Not Found"
        data["writedata"] = "HTTP/1.1 404 Not Found\r\nContent-Length: %s\r\nConnection:keep-alive\r\n\r\n%s" % (len(res),res)
        pass
    try:
        epoll_fd.modify(fd, select.EPOLLOUT | select.EPOLLIN | select.EPOLLERR | select.EPOLLHUP)
    except Exception, e:
        #print str(e)+getTraceStackMsg()
        pass

class Worker(object):

    def __init__(self):
        self._obj_dict = {}
        self._mtime_dict = {}
        self.httpparse = QuickHTTPRequest()
        self.log = FeimatLog("access.log")

        for l in listfile:
            if l == str(__file__):
                continue
            key, extname = os.path.splitext(l)
            if extname == ".py" and key in sys.modules:
                try:
                    action = sys.modules[key]
                    self._obj_dict[key] = eval("action.%s()" % key)
                except Exception, e:
                    #print str(e)+getTraceStackMsg()
                    continue
                self._mtime_dict[key] = action_time[key]

    def getGloalAction(self, action_key):
        action = sys.modules.get(action_key, None)
        auto_update = False
        if action == None:
            auto_update = True
        else:
            try:
                auto_update = action.FastpyAutoUpdate
            except Exception, e:
                pass
        if not auto_update:
            return auto_update,None,None
        if action == None:
            action = __import__(action_key)
            mtime = os.path.getmtime("./%s.py" % action_key)
            action_time[action_key] = mtime
        else:
            load_time = action_time[action_key]
            mtime = os.path.getmtime("./%s.py" % action_key)
            if mtime>load_time:
                try:
                    del sys.modules[action_key]
                    del action
                except Exception, e:
                    pass
                action = __import__(action_key)
                action_time[action_key] = mtime
        return auto_update,action,mtime

    def process(self, data, epoll_fd, fd):
        res = ""
        add_head = ""
        try:
            self.httpparse.parse(data)
            request = self.httpparse
        except Exception, e:
            print str(e)+getTraceStackMsg()
            res = "http format error"
        try:
            headers = {}
            headers["Content-Type"] = "text/html;charset=utf-8"
            headers["Connection"] = "keep-alive"
            if request.baseuri == "/favicon.ico":
                request.baseuri = "/"+static_file_dir+request.baseuri
            if static_dir in request.baseuri or "favicon.ico" in request.baseuri:
                sendfilejob(self.log,request,data,epoll_fd,fd)
                return None
            action_key = request.action
            obj = self._obj_dict.get(action_key, None)
            load_time = self._mtime_dict.get(action_key, None)
            auto_update,action,mtime = self.getGloalAction(action_key)
            if auto_update and (obj == None or load_time == None or mtime>load_time):
                self._mtime_dict[action_key] = mtime
                obj = eval("action.%s()" % action_key)
                self._obj_dict[action_key] = obj

            method = getattr(obj, request.method)
            res = method(request, headers)
            if headers.get("Content-Encoding","") == "gzip":
                buf = StringIO()
                f = gzip.GzipFile(mode='wb', fileobj=buf)
                f.write(res)
                f.close()
                res = buf.getvalue()
            self.log.log(" success: %s" % (request.path))
        except Exception, e:
            self.log.log(" fail: %s %s" % (request.path, str(e)+getTraceStackMsg()))
            res = "404 Not Found"
        try:
            if headers.get("Connection","") != "close":
                data["keepalive"] = True
            res_len = len(res)
            headers["Content-Length"] = res_len
            for key in headers:
                add_head += "%s: %s\r\n" % (key, headers[key])
            if res == "404 Not Found":
                data["writedata"] = "HTTP/1.1 404 Not Found\r\n%s\r\n%s" % (add_head, res)
            else:
                data["writedata"] = "HTTP/1.1 200 OK\r\n%s\r\n%s" % (add_head, res)
            if "read_cache_name" in data:
                os.remove(data["read_cache_name"])
                del data["read_cache_name"]
            epoll_fd.modify(fd, select.EPOLLOUT | select.EPOLLIN | select.EPOLLERR | select.EPOLLHUP)
        except Exception, e:
            print str(e)+getTraceStackMsg()


def InitLog():
    logger.setLevel(logging.DEBUG)
    fh = logging.FileHandler("network-server.log")
    fh.setLevel(logging.DEBUG)
    ch = logging.StreamHandler()
    ch.setLevel(logging.ERROR)
    formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
    ch.setFormatter(formatter)
    fh.setFormatter(formatter)
    logger.addHandler(fh)
    logger.addHandler(ch)


class MyThread(threading.Thread):
    ind = 0
    def __init__(self, threadCondition, shareObject, **kwargs):
        threading.Thread.__init__(self, kwargs=kwargs)
        self.threadCondition = threadCondition
        self.shareObject = shareObject
        self.setDaemon(True)
        self.worker = Worker()


    def processer(self, param, epoll_fd, fd):
        try:
            self.worker.process(param, epoll_fd, fd)
        except:
            print  "job error:" + getTraceStackMsg()


    def run(self):
        while True:
            try:
                param, epoll_fd, fd = self.shareObject.get()
                self.processer(param, epoll_fd, fd)
            except Queue.Empty:
                continue
            except :
                print "thread error:" + getTraceStackMsg()


class ThreadPool:
    def __init__( self, num_of_threads=10):
        self.threadCondition=threading.Condition()
        self.shareObject=Queue.Queue()
        self.threads = []
        self.__createThreadPool( num_of_threads )


    def __createThreadPool( self, num_of_threads ):
        for i in range( num_of_threads ):
            thread = MyThread( self.threadCondition, self.shareObject)
            self.threads.append(thread)


    def start(self):
        for thread in self.threads:
            thread.start()


    def add_job( self, param, epoll_fd, fd ):
        self.shareObject.put( (param, epoll_fd, fd) )

def check_next_http(param, tp, epoll_fd, fd, work):
    datas = param.get("readdata", "")
    if "" == datas:
        param["toprocess"] = ""
        return 0
    read_len = len(datas)
    contentlen = param.get("contentlen", -1)
    headlen = param.get("headlen", -1)
    if contentlen == -1:
        len_s = datas.find("Content-Length:")
        if len_s < 0:
            len_s = datas.lower().find("content-length:")
        if len_s > 0:
            len_e = datas.find("\r\n", len_s)
        if len_s > 0 and len_e > 0 and len_e > len_s+15:
            len_str = datas[len_s+15:len_e].strip()
            if len_str.isdigit():
                contentlen = int(datas[len_s+15:len_e].strip())
                param["contentlen"] = contentlen
    if headlen == -1:
        headend = datas.find("\r\n\r\n")
        if headend > 0:
            headlen = headend + 4
            param["headlen"] = headlen
    if (contentlen >= 0 and headlen > 0 and (contentlen + headlen) <= read_len) or \
           (contentlen == -1 and headlen > 0 and headlen <= read_len):
        one_http_len = headlen
        if contentlen > 0:
            one_http_len += contentlen
        param["toprocess"] = param["readdata"][0:one_http_len]
        param["readdata"] = param["readdata"][one_http_len:read_len]
        read_len = read_len - one_http_len
        param["contentlen"] = -1
        param["headlen"] = -1
        param["read_len"] = read_len
        tp.add_job(param,epoll_fd,fd)
        #work.process(param,epoll_fd,fd)
        return one_http_len
    else:
        param["toprocess"] = ""
        return 0

def clearfd(epoll_fd, params, fd):
    try:
        epoll_fd.unregister(fd)
    except Exception, e:
        print str(e)+getTraceStackMsg()
        pass

    try:
        param = params[fd]
        param["connections"].close()
        f = param.get("f", None)
        if f != None:
            f.close()
        rc = param.get("rc", None)
        if rc != None:
            rc.close()
        if "read_cache_name" in param:
            os.remove(param["read_cache_name"])
    except Exception, e:
        print str(e)+getTraceStackMsg()
        pass

    try:
        del params[fd]
        logger.error(getTraceStackMsg())
        logger.error("clear fd:%s" % fd)
    except Exception, e:
        print str(e)+getTraceStackMsg()
        pass

def run_main(listen_fd):
    try:
        epoll_fd = select.epoll()
        epoll_fd.register(listen_fd.fileno(), select.EPOLLIN | select.EPOLLERR | select.EPOLLHUP)
    except select.error, msg:
        logger.error(msg)


    tp = ThreadPool(8)
    tp.start()
    work = Worker()

    params = {}


    last_min_time = -1
    while True:
        epoll_list = epoll_fd.poll()

        cur_time = time.time()
        for fd, events in epoll_list:
            if fd == listen_fd.fileno():
                while True:
                    try:
                        conn, addr = listen_fd.accept()
                        conn.setblocking(0)
                        epoll_fd.register(conn.fileno(), select.EPOLLIN | select.EPOLLERR | select.EPOLLHUP)
                        conn.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                        #conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, True)
                        params[conn.fileno()] = {"addr":addr,"writelen":0, "connections":conn, "time":cur_time}
                    except socket.error, msg:
                        break
            elif select.EPOLLIN & events:
                param = params.get(fd,None)
                if param == None:
                    continue
                param["time"] = cur_time
                datas = param.get("readdata","")
                cur_sock = params[fd]["connections"]
                read_len = param.get("read_len", 0)
                while True:
                    try:
                        data = cur_sock.recv(102400)
                        if not data:
                            clearfd(epoll_fd,params,fd)
                            break
                        else:
                            datas += data
                            read_len += len(data)
                    except socket.error, msg:
                        if msg.errno == errno.EAGAIN:
                            param["read_len"] = read_len
                            len_s = -1
                            len_e = -1
                            contentlen = param.get("contentlen", -1)
                            headlen = param.get("headlen", -1)
                            if contentlen == -1:
                                len_s = datas.find("Content-Length:")
                                if len_s < 0:
                                    len_s = datas.lower().find("content-length:")
                                if len_s > 0:
                                    len_e = datas.find("\r\n", len_s)
                                if len_s > 0 and len_e > 0 and len_e > len_s+15:
                                    len_str = datas[len_s+15:len_e].strip()
                                    if len_str.isdigit():
                                        contentlen = int(datas[len_s+15:len_e].strip())
                                        param["contentlen"] = contentlen
                            if contentlen > MaxReadSize:
                                clearfd(epoll_fd,params,fd)
                                break
                            if headlen == -1:
                                headend = datas.find("\r\n\r\n")
                                if headend > 0:
                                    headlen = headend + 4
                                    param["headlen"] = headlen
                            if ": multipart/form-data; boundary" in datas and \
                               len(datas) > 1024*1024*3 and "rc" not in param:
                                if headlen > 0 and contentlen > 0:
                                    param["rc"] = open("%s/%s.tmp" % (read_cache_dir,fd), "w")
                                    pass
                                else:
                                    clearfd(epoll_fd,params,fd)
                                    break
                            if "rc" in param:
                                param["rc"].write(datas)
                                param["readdata"] = ""
                            else:
                                param["readdata"] = datas
                            toprocess = param.get("toprocess", "")
                            if "" == toprocess and \
                                    ((contentlen >= 0 and headlen > 0 and (contentlen + headlen) <= read_len) or \
                                    (contentlen == -1 and headlen > 0 and headlen <= read_len)):
                                if "rc" in param:
                                    param["rc"].close()
                                    param["read_cache_name"] = "%s/%s.tmp" % (read_cache_dir,fd)
                                    param["rc"] = open(param["read_cache_name"], "r")
                                    read_len = 0
                                else:
                                    one_http_len = headlen
                                    if contentlen > 0:
                                        one_http_len += contentlen
                                    param["toprocess"] = param["readdata"][0:one_http_len]
                                    param["readdata"] = param["readdata"][one_http_len:]
                                    read_len = read_len - one_http_len
                                    #logger.error(json.dumps(param["addr"])+param["toprocess"])
                                param["contentlen"] = -1
                                param["headlen"] = -1
                                param["read_len"] = read_len
                                tp.add_job(param,epoll_fd,fd)
                                #work.process(param,epoll_fd,fd)
                            break
                        else:
                            clearfd(epoll_fd,params,fd)
                            break
            elif select.EPOLLHUP & events or select.EPOLLERR & events:
                clearfd(epoll_fd,params,fd)
                logger.error("sock: %s error" % fd)
            elif select.EPOLLOUT & events:
                param = params.get(fd,None)
                if param == None:
                    continue
                param["time"] = cur_time
                sendLen = param.get("writelen",0)
                writedata = param.get("writedata", "")
                total_write_len = len(writedata)
                cur_sock = param["connections"]
                f = param.get("f", None)
                totalsenlen = param.get("totalsenlen", None)
                if writedata == "":
                    clearfd(epoll_fd,params,fd)
                    continue
                while True:
                    try:
                        sendLen += cur_sock.send(writedata[sendLen:])
                        if sendLen == total_write_len:
                            if f != None and totalsenlen != None:
                                readmorelen = 102400
                                if readmorelen > totalsenlen:
                                    readmorelen = totalsenlen
                                morefiledata = ""
                                if readmorelen > 0:
                                    morefiledata = f.read(readmorelen)
                                if morefiledata != "":
                                    writedata = morefiledata
                                    sendLen = 0
                                    total_write_len = len(writedata)
                                    totalsenlen -= total_write_len
                                    param["writedata"] = writedata
                                    param["totalsenlen"] = totalsenlen
                                    continue
                                else:
                                    f.close()
                                    del param["f"]
                                    del param["totalsenlen"]
                            if param.get("keepalive", True):
                                param["writedata"] = ""
                                param["writelen"] = 0
                                epoll_fd.modify(fd, select.EPOLLIN | select.EPOLLERR | select.EPOLLHUP)
                                check_next_http(param, tp, epoll_fd, fd, work)
                            else:
                                clearfd(epoll_fd,params,fd)
                            break
                    except socket.error, msg:
                        if msg.errno == errno.EAGAIN:
                            param["writelen"] = sendLen
                            break
                        clearfd(epoll_fd,params,fd)
                        break
            else:
                continue

        #check time out
        if cur_time - last_min_time > 600:
            last_min_time = cur_time
            objs = params.items()
            for (key_fd,value) in objs:
                fd_time = value.get("time", 0)
                del_time = cur_time - fd_time
                if del_time > 600:
                    clearfd(epoll_fd,params,key_fd)
                elif fd_time < last_min_time:
                    last_min_time = fd_time


if __name__ == "__main__":
    reload(sys)
    sys.setdefaultencoding('utf8')
    InitLog()
    port = int(sys.argv[1])
    try:
        listen_fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
    except socket.error, msg:
        logger.error("create socket failed")
    try:
        listen_fd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    except socket.error, msg:
        logger.error("setsocketopt SO_REUSEADDR failed")
    try:
        listen_fd.bind(('', port))
    except socket.error, msg:
        logger.error("bind failed")
    try:
        listen_fd.listen(10240)
        listen_fd.setblocking(0)
    except socket.error, msg:
        logger.error(msg)


    child_num = 7
    c = 0
    while c < child_num:
        c = c + 1
        newpid = os.fork()
        if newpid == 0:
            run_main(listen_fd)
    run_main(listen_fd)
