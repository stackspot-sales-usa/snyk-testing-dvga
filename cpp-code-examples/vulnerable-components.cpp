#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509.h>

void initialize_ssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_ssl() {
    EVP_cleanup();
}

int main() {
    initialize_ssl();

    const char* hostname = "www.example.com";
    const char* port = "443";

    SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());
    if (!ctx) {
        std::cerr << "Unable to create SSL context" << std::endl;
        ERR_print_errors_fp(stderr);
        return 1;
    }

    SSL* ssl;
    BIO* bio = BIO_new_ssl_connect(ctx);
    BIO_get_ssl(bio, &ssl);
    if (!ssl) {
        std::cerr << "Unable to get SSL pointer" << std::endl;
        ERR_print_errors_fp(stderr);
        return 1;
    }

    // Set the hostname and port
    BIO_set_conn_hostname(bio, hostname);
    BIO_set_conn_port(bio, port);

    // Attempt to connect
    if (BIO_do_connect(bio) <= 0) {
        std::cerr << "Error connecting to server" << std::endl;
        ERR_print_errors_fp(stderr);
        return 1;
    }

    // Attempt to perform SSL handshake
    if (BIO_do_handshake(bio) <= 0) {
        std::cerr << "Error performing SSL handshake" << std::endl;
        ERR_print_errors_fp(stderr);
        return 1;
    }

    // Vulnerability: Not validating the server's SSL certificate
    X509* cert = SSL_get_peer_certificate(ssl);
    if (cert) {
        std::cout << "Server certificate:" << std::endl;
        char* line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        std::cout << "Subject: " << line << std::endl;
        OPENSSL_free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        std::cout << "Issuer: " << line << std::endl;
        OPENSSL_free(line);
        X509_free(cert);
    } else {
        std::cerr << "No server certificate presented" << std::endl;
    }

    // Send a simple HTTP GET request
    const char* request = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n";
    BIO_write(bio, request, strlen(request));

    // Read the response
    char buffer[1024];
    while (true) {
        int bytes = BIO_read(bio, buffer, sizeof(buffer));
        if (bytes <= 0) break;
        std::cout.write(buffer, bytes);
    }

    // Clean up
    BIO_free_all(bio);
    SSL_CTX_free(ctx);
    cleanup_ssl();

    return 0;
}