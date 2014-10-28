/**
 * @file
 *
 * (c) CossackLabs
 */

#include <common/error.h>
#include <soter/soter.h>
#include <soter/soter_rsa_key.h>
#include "soter_openssl.h"
#include "soter_rsa_common.h"
#include <openssl/evp.h>
#include <openssl/rsa.h>

soter_status_t soter_verify_init_rsa_pss_pkcs8(soter_sign_ctx_t* ctx, const void* private_key, const size_t private_key_length, const void* public_key, const size_t public_key_length)
{
  /* pkey_ctx init */
  EVP_PKEY *pkey;
  pkey = EVP_PKEY_new();
  if (!pkey){
    return HERMES_NO_MEMORY;
  }
  if (!EVP_PKEY_set_type(pkey, EVP_PKEY_RSA)){
    EVP_PKEY_free(pkey);
    return HERMES_FAIL;
  }
  ctx->pkey_ctx = EVP_PKEY_CTX_new(pkey, NULL);
  if (!(ctx->pkey_ctx)){
    EVP_PKEY_free(pkey);
    return HERMES_FAIL;
  }
  if(soter_rsa_import_key(pkey, private_key, private_key_length)!=HERMES_SUCCESS){
    EVP_PKEY_free(pkey);
    return HERMES_FAIL;
  }
  if(soter_rsa_import_key(pkey, public_key, public_key_length)!=HERMES_SUCCESS){
    EVP_PKEY_free(pkey);
    return HERMES_FAIL;
  }

  /*md_ctx init*/
  ctx->md_ctx = EVP_MD_CTX_create();
  if(!(ctx->md_ctx)){
    EVP_PKEY_CTX_free(ctx->pkey_ctx);
    return HERMES_NO_MEMORY;
  }
  if(!EVP_DigestVerifyInit(ctx->md_ctx, &(ctx->pkey_ctx), EVP_sha256(), NULL, pkey)){
    EVP_PKEY_CTX_free(ctx->pkey_ctx);
    return HERMES_FAIL;
  }
  if(!EVP_PKEY_CTX_set_rsa_padding(ctx->pkey_ctx, RSA_PKCS1_PSS_PADDING)){
    EVP_PKEY_CTX_free(ctx->pkey_ctx);
    return HERMES_FAIL;
  }
  if(!EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx->pkey_ctx, -2)){
    EVP_PKEY_CTX_free(ctx->pkey_ctx);
    return HERMES_FAIL;
  }  
  return HERMES_SUCCESS;
}

soter_status_t soter_verify_update_rsa_pss_pkcs8(soter_sign_ctx_t* ctx, const void* data, const size_t data_length)
{
  if(!EVP_DigestVerifyUpdate(ctx->md_ctx, data, data_length)){
    return HERMES_FAIL;
  }
  return HERMES_SUCCESS;
}

soter_status_t soter_verify_final_rsa_pss_pkcs8(soter_sign_ctx_t* ctx, void* signature, size_t signature_length)
{
  EVP_PKEY *pkey = EVP_PKEY_CTX_get0_pkey(ctx->pkey_ctx);
  if (!pkey){
    return HERMES_INVALID_PARAMETER;
  }
  if(signature_length!=EVP_PKEY_size(pkey)){
    return HERMES_FAIL;
  }
  if(!EVP_DigestVerifyFinal(ctx->md_ctx, signature, signature_length)){
    return HERMES_FAIL;
  }
  return HERMES_SUCCESS;
}
