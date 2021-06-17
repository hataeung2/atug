#ifndef _ATUG2_CRYPTOT_H_
#define _ATUG2_CRYPTOT_H_
#include "atug2.h"

#include "cryptlib.h"
#include "Base64.h"
#include "aes.h"        
#include "seed.h"
#include "des.h"
#include "sha.h"
#include "modes.h"      
#include "filters.h"
#include "hex.h"

using namespace CryptoPP;
#if defined (_WINDOWS)
#if defined (_DEBUG)
#pragma comment(lib, "cryptlibd")
#else 
#pragma comment(lib, "cryptlib")
#endif
#endif
namespace atug2 {
  // helper func
  static const string StringToHex(const string& _input) {
    const string& input{ _input };
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
      const unsigned char c = input[i];
      output.push_back(lut[c >> 4]);
      output.push_back(lut[c & 15]);
    }
    return output;
  }
  static const string Sha256(const string& _src) {
    const string& plain_text{ _src };
    CryptoPP::byte const* data = (CryptoPP::byte*)plain_text.data();
    unsigned len = plain_text.length();
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256().CalculateDigest(digest, data, len);
    return StringToHex(string((char*)digest, CryptoPP::SHA256::DIGESTSIZE));
  }
  static const string Sha1(const string& _src) {
    CryptoPP::SHA1 sha1;
    string hash{};
    CryptoPP::StringSource(
      _src, 
      true, 
      new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash)))
    );
    return hash;
  }
  //!helper func


  //@sample
  /*using namespace acrypto;
  wstring plain_text = "plain text";
  byte KEY[Aaes::DEFAULT_KEYLENGTH] = { 3, };
  byte IV[Aaes::BLOCKSIZE] = { 0x04, };
  wstring encoded_text = ACrypto::CbcEncrypt<Aaes>(KEY, IV, plain_text);
  wstring recovered_text = ACrypto::CbcDecrypt<Aaes>(KEY, IV, encoded_text);
  */

  typedef CryptoPP::SEED Aseed;
  typedef CryptoPP::AES Aaes;
  typedef CryptoPP::DES Ades;

  class ACrypto {
  public:
    template <typename T>
    static string CbcEncrypt(byte* _key, byte* _iv, const string& _plain_text);
    template <typename T>
    static string CbcDecrypt(byte* _key, byte* _iv, const string& _encoded_text);
    template <typename T>
    static string EcbEncrypt(byte* _key, const string& _plain_text);
    template <typename T>
    static string EcbDecrypt(byte* _key, const string& _encoded_text);
  private:
    template <typename ModeType>
    static string Encrypt(ModeType& _encryptor, const string& _plain_text);
    template <typename ModeType>
    static string Decrypt(ModeType& _decryptor, const string& _encoded_text);
  };

  template<typename T>
  string ACrypto::CbcEncrypt(byte* _key, byte* _iv, const string& _plain_text) {
    typename CryptoPP::CBC_Mode<T>::Encryption encryptor(_key, T::DEFAULT_KEYLENGTH, _iv);
    return Encrypt(encryptor, _plain_text);
  }
  template<typename T>
  string ACrypto::CbcDecrypt(byte* _key, byte* _iv, const string& _encoded_text) {
    typename CryptoPP::CBC_Mode<T>::Decryption decryptor(_key, T::DEFAULT_KEYLENGTH, _iv);
    return Decrypt(decryptor, _encoded_text);
  }
  template<typename T>
  string ACrypto::EcbEncrypt(byte* _key, const string& _plain_text) {
    typename CryptoPP::ECB_Mode<T>::Encryption encryptor(_key, T::DEFAULT_KEYLENGTH);
    return Encrypt(encryptor, _plain_text);
  }
  template<typename T>
  string ACrypto::EcbDecrypt(byte* _key, const string& _encoded_text) {
    typename CryptoPP::ECB_Mode<T>::Decryption decryptor(_key, T::DEFAULT_KEYLENGTH);
    return Decrypt(decryptor, _encoded_text);
  }

  template<typename ModeType>
  string ACrypto::Encrypt(ModeType& _encryptor, const string& _plain_text) {
    string encoded_text;
    try {
      CryptoPP::StringSource(_plain_text, true,
        new CryptoPP::StreamTransformationFilter(_encryptor,
          new CryptoPP::Base64Encoder(
            new CryptoPP::StringSink(encoded_text), false),
          CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING
        )
      );
    }
    catch (...) {}
    return encoded_text;
  }

  template<typename ModeType>
  string ACrypto::Decrypt(ModeType& _decryptor, const string& _encoded_text) {
    string recovered_text;
    try {
      CryptoPP::StringSource(_encoded_text, true,
        new CryptoPP::Base64Decoder(
          new CryptoPP::StreamTransformationFilter(_decryptor,
            new CryptoPP::StringSink(recovered_text),
            CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING
          )
        )
      );
    }
    catch (...) {}
    return recovered_text;
  }
}//!namespace atug2

#endif//!_ATUG2_CRYPTOT_H_
