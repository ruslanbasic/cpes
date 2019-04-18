# -*- coding: utf-8 -*-

from BaseHTTPServer import BaseHTTPRequestHandler
import json
import base64

class StoreHandler(BaseHTTPRequestHandler):

    def do_POST(self):
        if self.path.startswith('/v1.0/sign_in/device'):
            length = self.headers['content-length']
            data = self.rfile.read(int(length))

            print data

            self.send_response(200)
            self.end_headers()
            resp = json.dumps({
                "status":"success",
                "error_code":0,
                "error_message":"no errors",
                "data":{"device_id":2,"authkey":"RYnfdRto_SJ5i7CAou6HzESU62VqrCZ7"}}, ensure_ascii=False)
            resp = base64.b64encode(resp)
            self.wfile.write(resp)


if __name__ == '__main__':
    from BaseHTTPServer import HTTPServer
    server = HTTPServer(('', 0), StoreHandler)
    sa = server.socket.getsockname()
    print 'Starting server at http://{}:{}'.format(sa[0], sa[1])
    server.serve_forever()
