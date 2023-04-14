
int base64_encoder(char *in, int in_size, char *out, int out_size);
int text2base64_size(int len);
int base64_decoder(const char *in, int in_size, char *out, int out_size);
int base642text_size(const char *code,int len);