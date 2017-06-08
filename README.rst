Dpac
====

Dpac is a dynamic proxy auto configuration CGI application. It returns
a Javascript proxy configuration dynamically based on the client's IP
address.

How does it work?
-----------------

It's a CGI application, which means it's invoked by a web server. When web 
browsers send a request for their proxy configuration, dpac looks at the
source IP address and responds with a proxy auto configuration file that
matches the network and filename specified in a configuration file.

This makes it simple and easy to route traffic to different proxies or a
cluster of proxies based on a client's IP address.

How do I use it?
----------------

For example, you could configure all of the browsers on your network to
request the proxy configuration from ``http://pac.example.com/proxy.pac``.
You'd then create the files below:

1) The configuration file, ending in ".conf": proxy.pac.conf

This file contains the list of networks that are mapped to the data
files that contain your Javascript function, like this:
::
    10.0.0.0/9    proxy1.js
    10.128.0.0/9  proxy2.js
    *             proxy1.js

The asterisk character ("*") matches all IP addresses. Therefore, it's
intended use is as a wildcard, or default, line in the conf file that
applies to all clients. It should typically be the last line; since it
matches all addresses, every line below it will be ignored (i.e. the
first match wins and processing stops).

2) The proxy auto configuration data files referenced in the conf file.

These files should contain the Javascript function "FindProxyForURL" that
contains the proxy selection logic you want to return to the browser.
Continuing the example above, you'd need to create 2 data files:

``proxy1.js``:

.. code-block:: javascript

    function FindProxyForURL(url, host)
    {
        if (isPlainHostName(host)) {
            return "DIRECT";
        } else if (dnsDomainIs(host, ".example.com")) {
            return "DIRECT";
        } else if (isInNet(host, "10.0.0.0", "255.0.0.0")) {
            return "DIRECT";
        } else if (isInNet(host, "172.16.0.0", "255.240.0.0")) {
            return "DIRECT";
        } else if (isInNet(host, "192.168.0.0", "255.255.0.0")) {
            return "DIRECT";
        } else if (isInNet(host, "127.0.0.0", "255.0.0.0")) {
            return "DIRECT";
        } else if (isInNet(host, myIpAddress(), "255.255.255.255")) {
            return "DIRECT";
        } else {
            return "PROXY proxy1.example.com:3128; PROXY proxy1b.example.com:3128";
        }
    }

``proxy2.js``:

.. code-block:: javascript

    function FindProxyForURL(url, host)
    {
        if (isPlainHostName(host)) {
            return "DIRECT";
        } else if (dnsDomainIs(host, ".example.com")) {
            return "DIRECT";
        } else if (isInNet(host, "10.0.0.0", "255.0.0.0")) {
            return "DIRECT";
        } else if (isInNet(host, "172.16.0.0", "255.240.0.0")) {
            return "DIRECT";
        } else if (isInNet(host, "192.168.0.0", "255.255.0.0")) {
            return "DIRECT";
        } else if (isInNet(host, "127.0.0.0", "255.0.0.0")) {
            return "DIRECT";
        } else if (isInNet(host, myIpAddress(), "255.255.255.255")) {
            return "DIRECT";
        } else {
            return "PROXY proxy2.example.com:3128; PROXY proxy2b.example.com:3128";
        }
    }

Configure your web server (Apache, NGINX, etc.) to run ``dpac`` as a CGI application and copy the files above
to the configured directory. Be sure to follow any best practices for security for your web server when configuring
it for CGI.

Build the ``dpac`` binary if needed (see the `Makefile <https://github.com/natej/dpac/blob/master/src/Makefile>`_
and `bin <https://github.com/natej/dpac/blob/master/bin/>`_ directory) and
copy it to your web server's configured CGI directory. Rename ``dpac`` to ``proxy.pac`` or whatever name matches your
proxy auto configuration URL. When run, it will look in its current directory for a filename matching itself with
the ``.conf`` extension. For example, if ``dpac`` is renamed to ``proxy.pac``, it will look in its current directory
for a configuration file named ``proxy.pac.conf``.

See the `www <https://github.com/natej/dpac/blob/master/www/>`_ directory for an example.
