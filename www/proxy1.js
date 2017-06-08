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
