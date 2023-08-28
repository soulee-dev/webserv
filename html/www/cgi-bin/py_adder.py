import os
import urllib.parse
import sys

def parse_cgi_arguments(uri_args):
    params = urllib.parse.parse_qs(uri_args)
    return {k: v[0] if v else None for k, v in params.items()}


buf = os.environ.get("QUERY_STRING")
args = parse_cgi_arguments(buf)

a = args.get("fnum", 0)
b = args.get("snum", 0)
result = int(a) + int(b)

html_template = """QUERY_STRING={}
Welcome to py_adder.com:
<p>THE Internet addition portal.</p>
<p>The answer is: {} + {} = {}</p>
Thanks for visiting!"""

html = html_template.format(buf, a, b, result)

# header_template = """Connection: close\r\nContent-length: {}\r\nContent-type: text/html\r\n\r\n"""
header_template = """Connection: close\r\nContent-type: text/html\r\n\r\n"""

header = header_template.format(len(html))

# print(header, end="")
print(header_template)
print(html)
sys.exit()
