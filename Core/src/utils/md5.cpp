#include "utils/md5.h"
#include <windows.h>
#include <wincrypt.h>
#include <iomanip>
#include <sstream>
#include <vector>

std::string computeMD5(const std::vector<char>& data) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    std::string result = "";

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) return "";
    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) { CryptReleaseContext(hProv, 0); return ""; }
    if (!CryptHashData(hHash, (BYTE*)(data.data()), data.size(), 0)) {
        CryptDestroyHash(hHash); CryptReleaseContext(hProv, 0); return "";
    }

    DWORD cbHashSize = 16;
    DWORD dwCount = sizeof(DWORD);
    BYTE rgbHash[16];
    
    if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHashSize, 0)) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (DWORD i = 0; i < cbHashSize; i++) {
            oss << std::setw(2) << (int)rgbHash[i];
        }
        result = oss.str();
    }
    
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return result;
}
