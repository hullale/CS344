all: build
	
clean:
	rm -f otp_dec
	rm -f otp_dec_d
	rm -f otp_enc
	rm -f otp_enc_d
	rm -f keygen
	
build:
	gcc otp_dec.c -o otp_dec -std=c99
	gcc otp_dec_d.c -o otp_dec_d -std=c99
	gcc otp_enc.c -o otp_enc -std=c99
	gcc otp_enc_d.c -o otp_enc_d -std=c99
	gcc keygen.c -o keygen -std=c99