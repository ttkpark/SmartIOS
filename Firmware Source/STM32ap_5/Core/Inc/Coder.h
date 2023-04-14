uint8_t* addr_IV(int code);
uint8_t* addr_Key(int code);
void AESEncode(uint8_t* input_output,int len,int code);
void AESDecode(uint8_t* input_output,int len,int code);


void CoderBuffer_deinit(CoderBuffer buffer);
void CoderBuffer_free(CoderBuffer buffer);
Result CoderBuffer_malloc(CoderBuffer* bufptr,LENGTH len);
CoderBuff   CoderBuffer_make(uint8_t *data,int len);
CoderBuffer CoderBuffer_clone(uint8_t *data,int len);
void CoderBuffer_print(CoderBuffer buffer,int line);
void CoderBuffer_print_nonpointer(CoderBuff buffer,int line);

void makepadding(uint8_t* dest,uint8_t* data,int dest_len,int data_len);

CoderBuffer Coder_AESEncode(CoderBuffer input,int code);
CoderBuffer Coder_AESDecode(CoderBuffer input,int code);
CoderBuffer Coder_Base64Encode(CoderBuffer input);
CoderBuffer Coder_Base64Decode(CoderBuffer input);

// Example of this
// char* text = "This is me!";
// CoderBuffer coder_text = CoderBuffer_clone(text,strlen(text));
// CoderBuffer *buffer = &coder_text;
// CoderBuffer_print(buffer,16);

// Debug("Base64Encode1");
// buffer = Coder_Base64Encode(buffer);
// if(buffer == NULL)return 0;
// CoderBuffer_print(buffer,16);

// Debug("Base64Encode2");
// buffer = Coder_Base64Encode(buffer);
// if(buffer == NULL)return 0;
// CoderBuffer_print(buffer,16);

// Debug("AESEncode");
// buffer = Coder_AESEncode(buffer,0);
// if(buffer == NULL)return 0;
// CoderBuffer_print(buffer,16);


// Debug("AESDecode");
// buffer = Coder_AESDecode(buffer,0);
// if(buffer == NULL)return 0;
// CoderBuffer_print(buffer,16);

// Debug("Base64Decode2");
// buffer = Coder_Base64Decode(buffer);
// if(buffer == NULL)return 0;
// CoderBuffer_print(buffer,16);

// Debug("Base64Decode1");
// buffer = Coder_Base64Decode(buffer);
// if(buffer == NULL)return 0;
// CoderBuffer_print(buffer,16);


// free(buffer);