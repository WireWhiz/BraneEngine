#pragma once

#include "acme-exception.h"

#include <list>
#include <memory>

namespace acme_lw
{

struct Certificate
{
    std::string fullchain;
    std::string privkey;

    // Note that neither of the 'Expiry' calls below require 'privkey'
    // to be set; they only rely on 'fullchain'.

    /**
        Returns the number of seconds since 1970, i.e., epoch time.

        Due to openssl quirkiness there might be a little drift
        from a strictly accurate result, but it should be close
        enough for the purpose of determining whether the certificate
        needs to be renewed.
    */
    long getExpiry() const;

    /**
        Returns the 'Not After' result that openssl would display if
        running the following command.

            openssl x509 -noout -in fullchain.pem  -text

        For example:

            May  6 21:15:03 2018 GMT
    */
    std::string getExpiryDisplay() const;
};

struct AcmeClientImpl;

class AcmeClient
{
public:
    /**
        The signingKey is the Acme account private key used to sign
        requests to the acme CA, in pem format.
    */
    AcmeClient(const std::string& signingKey);

    ~AcmeClient();

    /**
        The implementation of this function allows Let's Encrypt to
        verify that the requestor has control of the domain name.

        The callback may be called once for each domain name in the
        'issueCertificate' call. The callback should do whatever is
        needed so that a GET on the url returns the 'keyAuthorization',
        (which is what the Acme protocol calls the expected response.)

        Note that this function may not be called in cases where
        Let's Encrypt already believes the caller has control
        of the domain name.
    */
    typedef void (*Callback) (  const std::string& domainName,
                                const std::string& url,
                                const std::string& keyAuthorization);

    /**
        Issue a certificate for the domainNames. If there is more than one, the
        first one will be the 'Subject' (CN) in the certificate.

        throws std::exception, usually an instance of AcmeException
    */
    Certificate issueCertificate(const std::list<std::string>& domainNames, Callback);

    // Call once before instantiating AcmeClient. Not thread safe.
    static void init();

    // Call once before application shutdown. Not thread safe.
    static void teardown();

private:
    std::unique_ptr<AcmeClientImpl> impl_;
};

}
