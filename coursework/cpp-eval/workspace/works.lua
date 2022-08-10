wrk.body   = '{"operation":"touch","filename":"/dirfirst/dirsecon/dirthird/dirforth/file.txt"}'
wrk.method = "POST"
wrk.headers["Authorization"] = "Basic MjNiYzQ2YjEtNzFmNi00ZWQ1LThjNTQtODE2YWE0ZjhjNTAyOjEyM3pPM3haQ0xyTU42djJCS0sxZFhZRnBYbFBrY2NPRnFtMTJDZEFzTWdSVTRWck5aOWx5R1ZDR3VNREdJd1A"
wrk.headers["Content-Type"] = "application/json"
wrk.headers["User-Agent"] = "OpenWhisk-CLI/1.0 (2021-04-01T23:49:54.523+0000) linux amd64"

response = function(status, headers, body)
--    if status ~= 200 then
--        print(status, body)
--    end
    print(status, body)
end
