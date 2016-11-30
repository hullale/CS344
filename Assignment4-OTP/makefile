all: build
	
clean:
	rm -f otp_dec
	rm -f otp_dec_d
	rm -f otp_enc
	rm -f otp_enc_d
	rm -f keygen
	
build:
	gcc otp_dec.c -o otp_dec
	gcc otp_dec_d.c -o otp_dec_d
	gcc otp_enc.c -o otp_enc
	gcc otp_enc_d.c -o otp_enc_d
	gcc keygen.c -o keygen