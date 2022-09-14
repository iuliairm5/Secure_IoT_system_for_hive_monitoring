#define AES_TEXT_SIZE 16 // câți bytes va avea bufferul de date criptate
//declararea variabilelor
uint32_t semnal_temp_ghem_necriptat;
uint8_t id_ntc_ghem=1;
uint32_t semnal_fotodioda_necriptat;
uint8_t id_fotodioda=2;
uint8_t id_greutate=3;
uint32_t semnal_greutate_necriptat;
uint32_t vscap_adc;
float dac_referinta=1.25; //maxim 3.3V
uint8_t dac_ref_byte; //vreau pe 8 biți iesirea dac
uint32_t pachet_date_criptate[AES_TEXT_SIZE]; //se va trimite mai departe (pe serială sau radio)
//alte variabile necesare
ADC_ChannelConfTypeDef sConfig; //pentru a putea schimba canalul ADC-ului
LIS3DH_DataScaled myLISdata;//datele de la senzorului LIS3DH 
uint8_t datareadyflagLIS = 0; //un flag care arată că datele încă nu sunt pregătite a fi citite cu LIS3DH
----------------------------------------------------------------------------------------------------

//cod din main
//citirea datelor de la ADC
//deoarece am senzori pe diferite canale ale unui ADC, doresc să comut între ele la un interval anume de timp (voi începe mereu cu senzorul de pe canalul 3 deși el este canalul by default selectat de către Cube)
sConfig.Channel = ADC_CHANNEL_3; //schimb pe canalul 3, indiferent de ultimul canal selectat
HAL_ADC_ConfigChannel(&hadc2, &sConfig); //funcția care implementează configurația
HAL_ADC_Start(&hadc2); //pornesc ADC-ul
if(HAL_ADC_PollForConversion(&hadc2, 5) == HAL_OK) //aștept 5 ms iar dacă această conversie s-a realizat cu succes, abia atunci voi citi valorile cu ADC
{
semnal_temp_ghem_necriptat = HAL_ADC_GetValue(&hadc2); //salvez datele citite în variabila corespunzătoare
}
HAL_Delay(1000); //aștept 1 sec între citiri
sConfig.Channel = ADC_CHANNEL_4; //schimb pe canalul 4
HAL_ADC_ConfigChannel(&hadc2, &sConfig); 
HAL_ADC_Start(&hadc2); //repornesc iar ADC-ul
if(HAL_ADC_PollForConversion(&hadc2, 5) == HAL_OK) 
{semnal_fotodioda_necriptat = HAL_ADC_GetValue(&hadc2);}
HAL_Delay(1000);
sConfig.Channel = ADC_CHANNEL_13; //schimb pe canalul 4
HAL_ADC_ConfigChannel(&hadc2, &sConfig); 
HAL_ADC_Start(&hadc2); 
if(HAL_ADC_PollForConversion(&hadc2, 5) == HAL_OK) 
{vscap_adc = HAL_ADC_GetValue(&hadc2);//citesc valoarea VSCap cu care apoi pot face acea condiție IF
}
----------------------------------------------------------------------------------------------------
//comparatorul care lucrează în mod continuu și trezește MCU dacă sunt zgomote suspecte de la microfon 
dac_ref_byte = (uint8_t)((dac_referinta/3.3)*255); //fac conversie de la floating point (domeniul 0V-3.3V) la byte (domeniul 0 -255), altfel nu v-a compila tocmai bine
HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);//pornesc DAC-ul pe canalul 1
HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, dac_ref_byte); //am la ieșirea DAC tensiunea de referință de 1.25V pe 8 biți
HAL_COMP_Start(&hcomp1); //pornesc comparatorul
if(HAL_COMP_GetOutputLevel(&hcomp1) == COMP_OUTPUT_LEVEL_HIGH) //verific dacă tensiunea citită de la microfon > 1.25V
{ ...... //trezesc microcontroller-ul }

//citire LIS3DH cu I2C (am setat external line interrupt pe pinul PC13 iar apoi am activat întreruperea din NVIC)
//fac inițializarea senzorului
LIS3DH_InitTypeDef myconfigLIS; //definesc o variabilă de configurare
myconfigLIS.dataRate = LIS3DH_DATARATE_12_5; //citesc date la 12.5Hz deci la 1/12.5=0.08sec

myconfigLIS.fullScale = LIS3DH_FULLSCALE_4; //domeniul senzorului va fi de +/4g ( g este accelerația gravitațională; 1g=9.8m/s^2)
myconfigLIS.antiAliasingBW = LIS3DH_FILTER_BW_50;//lățimea de bandă anti-aliasing setată la 50Hz pentru a nu se updata prea repede (este suficientă pentru a elimina zgomotele de înaltă frecvență)
myconfigLIS.enableAxes = LIS3DH_XYZ_ENABLE; //selectez toate cele 3 axe
myconfigLIS.interruptEnable = true;
LIS3DH_Init(&hi2c2, &myconfigLIS); //implementez configurația
uint8_t FINDSLAVEADDRESS(){ //caut printre toate adresele de 8b de I2C adresa SLAVE
uint8_t i;
for(i=0;i<255;i++)
{if(HAL_I2C_IsDeviceReady(&hi2c2, i, 1, 10) == HAL_OK)//testez adresa SLAVE; încerc o singură dată cu timeout=10ms 
{ break;}
}
return i; //folosesc această adresă în funcțiile HAL_I2C_Master_Transmit sau Receive
}
if((LIS3DH_PollDRDY(1000) == true) || (datareadyflagLIS == 1)) //aștept 1 sec ca data să fie pregătită a fi citită sau am o întrerupere externă
{datareadyflagLIS = 0; //resetez la 0
myLISdata = LIS3DH_GetDataScaled(); //citesc datele în formatul mili g (1000mg=1g=9.8m/s^2)
//data de pe fiecare axă trebuie să fie între -1000mg și +1000mg însă mai pot calibra citirile
}
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)//realizează întreruperea externă cu LIS3DH
{UNUSED(GPIO_Pin);
datareadyflagLIS = 1;}
----------------------------------------------------------------------------------------------------
//CRIPTARE, tot în main
HAL_CRYP_Encrypt(&hcryp, &semnal_temp_ghem_necriptat, 4, &semnal_temp_ghem_criptat,10); //timeout 
//10ms;4bytes de date
HAL_CRYP_Encrypt(&hcryp, &semnal_fotodioda_necriptat, 4, &semnal_fotodioda_criptat,10);
HAL_CRYP_Encrypt(&hcryp, &semnal_greutate_necriptat, 4, &semnal_greutate_criptat, 10);
pachet_date_criptate[1]=id_ntc_ghem;//1Byte
pachet_date_criptate[2]=semnal_temp_ghem_criptat;//4Bytes
pachet_date_criptate[6]=id_fotodioda;//1Byte
pachet_date_criptate[7]=semnal_fotodioda_criptat;//4Bytes
//transmit pe serială spre pc datele criptate
HAL_UART_Transmit(&huart1, (uint8_t *)pachet_date_criptate, 8, 10); //10ms timeout
