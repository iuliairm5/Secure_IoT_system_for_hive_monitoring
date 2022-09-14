# Secure_IoT_system_for_hive_monitoring

-	the solution has 2 self-made PCBs (in Kicad) that communicate data with each other via radio waves; one PCB contains the main sensors (temperature, humidity, noise, weight and luminosity) and the STM32L5 microcontroller (containing the C/C++ software to achieve the entire communication and data encryption ) whereas the second PCB acts as a gateway between the first board and the computer (where the data should be decrypted);
-	the power supply (tested in the laboratory) of 3.3 Vcc for the circuits is given by the solar/ photovoltaic panels, a DC-DC Boost convertor, 2 supercapacitors and a Buck-Boost convertor;
-	for the cryptography part I use a hybrid encryption (symmetric & asymmetric) with the following flow: I send the public keys to the gateway, on computer I generate with RNG the AES key and I encrypt it using the received public keys ; on the main board I decrypt the AES key using the private keys and I encrypt the sensor’s data using the obtained AES key 

![placa](https://user-images.githubusercontent.com/113541254/190244558-22d21670-bd9b-4d22-a184-42fcc93d2563.jpg)
