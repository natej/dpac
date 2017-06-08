Dpac Development Notes
======================

Dpac's number one goal is speed and efficiency; to minimize the number of
instructions required to find the first matching network in the conf file. 
If this means the code is more difficult to read or less beautiful, then
that's the price paid. Of course we should strive to minimize this.
The code should be correct first. :)

Having stated the above, keep them in mind when reading below:

-   All memory allocations are not freed. By definition this is a
    short-lived application in terms of running time. It's a CGI application.
    It's not a long-running daemon or background service. As soon as it
    exits, the memory will be reclaimed by the OS for subsequent
    invocations and other applications.

-   If errors are encountered, the process should simply exit with a non-zero
    status code. This should trigger an error within the web server and cause
    it to return a 500-level error to the browser.

    This behavior is by design. We want the browser to get an HTTP error so
    there is no chance that HTTP headers are sent, a 200 OK is sent and
    then we somehow encounter an error that prevents us from actually returning
    the pac file data. In other words, we should have already determined the
    appropriate pac file and read it into a buffer so the only thing left to
    do is send the buffer to stdout.
