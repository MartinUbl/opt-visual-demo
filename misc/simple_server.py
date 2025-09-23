from http.server import SimpleHTTPRequestHandler, HTTPServer

# This is a very simple Python script to run a webserver able to serve WASM binaries
# make sure to start this server in the directory containing the html+wasm+js files

class CustomHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        print("Header request");
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        super().end_headers()

    def guess_type(self, path):
        if path.endswith('.wasm'):
            return 'application/wasm'
        else:
            return super().guess_type(path)

if __name__ == '__main__':
    server_address = ('', 8950)
    httpd = HTTPServer(server_address, CustomHandler)
    print('Server running at localhost:8950...')
    httpd.serve_forever()

