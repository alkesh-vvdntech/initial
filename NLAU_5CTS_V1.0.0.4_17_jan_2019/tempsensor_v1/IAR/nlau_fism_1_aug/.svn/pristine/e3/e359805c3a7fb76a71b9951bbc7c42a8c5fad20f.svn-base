/***********************************HEADER FILES************************************************/
#include "modem.h"
#include "modem_uart.h"
#include "main_system.h"
#include "debug.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "spi_flash.h"
#include "file_system.h"
#include "apn_config.h"


/**********************************************************************************************
 * Function Name :void write_apn_234()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 234
 **********************************************************************************************/
void write_apn_file()
{
  char apn_buff[3000];
  char total_apn_buff = 0;
  char apn_name[10];

 memset(apn_buff,'\0',sizeof(apn_buff));
 memset(apn_name,'\0',sizeof(apn_name));

 while(++total_apn_buff <= 26)
 {
   switch(total_apn_buff)
   {

    case 1:   apn_208(apn_buff);
              strcpy(apn_name,"apn_208");
              break;

    case 2:   apn_228(apn_buff);
              strcpy(apn_name,"apn_228");
              break;

    case 3:   apn_234(apn_buff);
              strcpy(apn_name,"apn_234");
              break;

    case 4:   apn_235(apn_buff);
              strcpy(apn_name,"apn_235");
              break;

    case 5:   apn_238(apn_buff);
              strcpy(apn_name,"apn_238");
              break;

    case 6:   apn_286(apn_buff);
              strcpy(apn_name,"apn_286");
              break;

    case 7:   apn_310(apn_buff);
              strcpy(apn_name,"apn_310");
              break;

    case 8:   apn_311(apn_buff);
              strcpy(apn_name,"apn_311");
              break;

    case 9:   apn_316(apn_buff);
              strcpy(apn_name,"apn_316");
              break;

    case 10:  apn_404(apn_buff);
              strcpy(apn_name,"apn_404");
              break;

    case 11:  apn_405(apn_buff);
              strcpy(apn_name,"apn_405");
              break;


    case 12:  apn_414(apn_buff);
              strcpy(apn_name,"apn_414");
              break;

    case 13:  apn_417(apn_buff);
              strcpy(apn_name,"apn_417");
              break;

    case 14:  apn_514(apn_buff);
              strcpy(apn_name,"apn_514");
              break;

    case 15:  apn_515(apn_buff);
              strcpy(apn_name,"apn_515");
              break;


    case 16:  apn_608(apn_buff);
              strcpy(apn_name,"apn_608");
              break;

    case 17:  apn_621(apn_buff);
              strcpy(apn_name,"apn_621");
              break;

    case 18:  apn_622(apn_buff);
              strcpy(apn_name,"apn_622");
              break;

    case 19:  apn_623(apn_buff);
              strcpy(apn_name,"apn_623");
              break;

    case 20:  apn_636(apn_buff);
              strcpy(apn_name,"apn_636");
              break;

    case 21:  apn_639(apn_buff);
              strcpy(apn_name,"apn_639");
              break;

    case 22:  apn_640(apn_buff);
              strcpy(apn_name,"apn_640");
              break;

    case 23:  apn_643(apn_buff);
              strcpy(apn_name,"apn_643");
              break;

    case 24:  apn_650(apn_buff);
              strcpy(apn_name,"apn_650");
              break;


    case 25:  apn_653(apn_buff);
              strcpy(apn_name,"apn_653");
              break;

    case 26:  apn_732(apn_buff);
              strcpy(apn_name,"apn_732");
              break;

     default:
                  break;

   }
   write_apn_config(apn_name,apn_buff,strlen(apn_buff));
   memset(apn_buff,'\0',sizeof(apn_buff));
   memset(apn_name,'\0',sizeof(apn_name));
 }

}

/**********************************************************************************************
 * Function Name :void read_apn(char *apn__name,char *apn_code)
 * Parameters :  void
 * Return : void
 * Description : APN codes from 234
 **********************************************************************************************/
void read_apn(char *apn__name,char *apn_code)
{
  unsigned char *ret = 0;
  unsigned char count = 0;
  unsigned char ret_val= 0;
  char apn_file_to_read[10];
  unsigned int file_size= 0;
  unsigned int byte_count = 0;
  unsigned char incomplete_apn = 2;
  char check_apn[32];

  char debug_buff[50];

  memset(apn_file_to_read,'\0',sizeof(apn_file_to_read));
  memset(check_apn,'\0',sizeof(check_apn));

  strcat(apn_file_to_read,"apn_");
  strcat(apn_file_to_read,apn__name);
  file_size = read_file_size(apn_file_to_read);

  for(byte_count = 0; byte_count < file_size; byte_count += BUFFER_MAX_SIZE-10)
  {
     read_apn_config(apn_file_to_read,byte_count);

     if(incomplete_apn == 1)
     {
       for(count = 2; count < 34;count++)
       {
            if(GSM_buffer[count] == '|')
            {
              incomplete_apn = 0;
              strcat(device_apn,check_apn);
              break;
            }
            check_apn[count-2] = GSM_buffer[count];
       }
     }
     ret = strstr(GSM_buffer,apn_code);
     if(ret)
     {
       memset(device_apn,'\0',sizeof(device_apn));
       for(count = 0; count < 34;count++)
       {
            if(ret[count+strlen(apn_code)+1] == '|')
            {
              incomplete_apn = 0;
              break;
            }
            else if((ret[count+strlen(apn_code)+1] == '\0') ||  (ret[count+strlen(apn_code)+1] == '\r') || (ret[count+strlen(apn_code)+1] == '\n'))
            {
              incomplete_apn = 1;
              break;
            }
            device_apn[count] = ret[count+strlen(apn_code)+1];
       }

     }
     if(incomplete_apn == 0)
     {
        break;
     }

  }

  memset(debug_buff,'\0',sizeof(debug_buff));
  sprintf(debug_buff,"\r\n\r\nfile_size is %d and updated_apn is %s\r\n",file_size,device_apn);
  debug_print(debug_buff);

}

/**********************************************************************************************
 * Function Name :void apn_228()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 228
 **********************************************************************************************/
void apn_228(char *buffer)
{

strcpy(buffer,"01=gprs.swisscom.ch|\
02=internet|\
03=internet|\
51=internet|\
53=internet.ch.upcmobile.com|\
54=data.lycamobile.ch|\
60=internet|\
99=internet|");

};

/**********************************************************************************************
 * Function Name :void apn_732()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 732
 **********************************************************************************************/
void apn_732(char *buffer)

{

strcpy(buffer,"01=movilexito.net.co|\
02=internet|\
12=internet.movistar.com.co|\
20=web.colombiamovil.com.co|\
101=internet.comcel.com.co|\
102=internet.movistar.com.co|\
103=web.colombiamovil.com.co|\
111=web.colombiamovil.com.co|\
123=internet.movistar.com.co|\
130=lte.avantel.com.co|\
142=www.une.net.co|\
154=web.vmc.net.co|\
176=directvnet.com.co|\
187=moviletb.net.co|");

}

/**********************************************************************************************
 * Function Name :void apn_653()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 653
 **********************************************************************************************/
void apn_653(char *buffer)

{

strcpy(buffer,"01=internet|\
10=mymtn.co.sz|");

}

/**********************************************************************************************
 * Function Name :void apn_238()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 238
 **********************************************************************************************/
void apn_238(char *buffer)

{

strcpy(buffer,"01=internet|\
02=internet|\
06=data.tre.dk|\
07=webdk.mundio.com|\
10=internet|\
12=data.lycamobile.dk|\
20=www.internet.mtelia.dk|\
30=web.orange.dk|\
77=internet|");

}
/**********************************************************************************************
 * Function Name :void apn_640()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 640
 **********************************************************************************************/
void apn_640(char *buffer)

{

strcpy(buffer,"01=internet|\
02=tigoweb|\
03=znet|\
04=internet|\
05=internet|\
06=internet|\
07=internet|\
08=internet|\
09=internet|\
11=internet|\
12=internet|\
13=internet|");

}

/**********************************************************************************************
 * Function Name :void apn_208()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 208
 **********************************************************************************************/
void apn_208(char *buffer)

{

strcpy(buffer,"00=orange|\
01=orange|\
02=orange|\
03=internet.sierrawireless.com|\
04=sisteer|\
05=globalstar|\
06=globalstar|\
07=globalstar|\
08=completel|\
09=sl2sfr|\
10=sl2sfr|\
11=sl2sfr|\
13=sl2sfr|\
14=free|\
15=free|\
16=free|\
17=bornsip|\
18=voxbone|\
20=mmsbouygtel.com|\
21=mmsbouygtel.com|\
22=netgprs.com|\
23=sl2sfr|\
24=internet.sierrawireless.com|\
25=data.lycamobile.fr|\
26=fnetnrj|\
27=internet66|\
28=astrium|\
29=orange|\
30=symamobile.com|\
31=webfr.mundio.com|\
88=mmsbouygtel.com|\
89=bcom|\
90=internet|\
91=orange|\
92=internet|\
93=tdf|\
94=halys|");

}

/**********************************************************************************************
 * Function Name :void apn_286()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 286
 **********************************************************************************************/
void apn_286(char *buffer)

{

strcpy(buffer,"01=internet|\
02=internet|\
03=internet|\
04=internet|");

}
/**********************************************************************************************
 * Function Name :void apn_417()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 417
 **********************************************************************************************/
void apn_417(char *buffer)

{

strcpy(buffer,"01=net.syriatel.com|\
02=internet|\
09=net.syriatel.com|");

}

/**********************************************************************************************
 * Function Name :void apn_622()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 622
 **********************************************************************************************/
void apn_622(char *buffer)

{

strcpy(buffer,"01=internet.td.zain.com|\
02=default|\
03=net.millicom.com|\
04=default|");

}

/**********************************************************************************************
 * Function Name :void apn_623()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 623
 **********************************************************************************************/
void apn_623(char *buffer)

{

strcpy(buffer,"01=moov|\
02=tlclwap|\
03=orangecawap|\
04=nationlink|");

}

/**********************************************************************************************
 * Function Name :void apn_650()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 650
 **********************************************************************************************/
void apn_650(char *buffer)

{

strcpy(buffer,"01=internet|\
10=internet|");

}

/**********************************************************************************************
 * Function Name :void apn_515()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 515
 **********************************************************************************************/
void apn_515(char *buffer)

{

strcpy(buffer,"01=internet.globe.com.ph|\
02=internet.globe.com.ph|\
03=internet|\
05=minternet|\
11=weroam|\
18=redinternet|\
24=internet.abs-cbn.com.ph|\
88=internet.globe.com.ph|");

}

/**********************************************************************************************
 * Function Name :void apn_414()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 414
 **********************************************************************************************/
void apn_414(char *buffer)

{

strcpy(buffer,"01=mptnet|\
05=Ooredoo|\
06=internet|");

}

/**********************************************************************************************
 * Function Name :void apn_234()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 234
 **********************************************************************************************/
void apn_234(char *buffer)
{

strcpy(buffer,"01=webuk.mundio.com|\
02=mobile.o2.co.uk|\
03=internet|\
07=internet|\
08=internet.btonephone.com|\
10=mobile.o2.co.uk|\
11=mobile.o2.co.uk|\
15=internet|\
16=mobile.talktalk.co.uk|\
20=three.co.uk|\
25=truphone.com|\
26=data.lycamobile.co.uk|\
30=everywhere|\
31=everywhere|\
32=everywhere|\
33=everywhere|\
34=everywhere|\
50=pepper|\
51=pepper|\
55=internet|\
58=3gpronto|\
76=btmobile.bt.com|\
86=everywhere|");

}

/**********************************************************************************************
 * Function Name :void apn_235()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 235
 **********************************************************************************************/
void apn_235(char *buffer)
{
  strcpy(buffer,"94=three.co.uk|");
}

/**********************************************************************************************
 * Function Name :void apn_636()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 636
 **********************************************************************************************/
void apn_636(char *buffer)
{

strcpy(buffer,"01=etc.com|");

}

/**********************************************************************************************
 * Function Name :void apn_608()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 608
 **********************************************************************************************/
void apn_608(char *buffer)
{
strcpy(buffer,"01=internet|\
02=web.sentel.com|\
03=expresso|");

}
/**********************************************************************************************
 * Function Name :void apn_514()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 514
 **********************************************************************************************/
void apn_514(char *buffer)
{

strcpy(buffer,"01=internet|\
02=internet|\
03=t-internet|");

}

/**********************************************************************************************
 * Function Name :void apn_643()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 643
 **********************************************************************************************/
void apn_643(char *buffer)
{

strcpy(buffer,"01=isp.mcel.mz|\
03=internet|\
04=internet|");

}
/**********************************************************************************************
 * Function Name :void apn_639()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 639
 **********************************************************************************************/
void apn_639(char *buffer)
{

strcpy(buffer,"02=safaricom|\
03=internet|\
05=Internet|\
07=bew.orange.co.ke|");

}
/**********************************************************************************************
 * Function Name :void apn_621()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 639
 **********************************************************************************************/
void apn_621(char *buffer)
{

strcpy(buffer,"01=internet.visafone.com.ng|\
20=www.ng.airtel.com|\
24=spectranet|\
25=internet.visafone.com.ng|\
26=lte.swift.com|\
27=internet|\
30=web.gprs.mtnnigeria.net|\
40=internet|\
50=gloflat|\
60=etisalat|\
99=starcoms|");

}

/**********************************************************************************************
 * Function Name :void apn_404()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 404
 **********************************************************************************************/
void apn_404(char *buffer)
{

strcpy(buffer,"01=www|\
02=airtelgprs.com|\
03=airtelgprs.com|\
04=internet|\
05=www|\
07=internet|\
09=smartnet|\
10=airtelgprs.com|\
11=www|\
12=internet|\
13=www|\
14=internet|\
15=www|\
16=airtelgprs.com|\
17=aircelgprs|\
18=rcomnet|\
19=internet|\
20=www|\
21=www|\
22=internet|\
24=internet|\
25=aircelgprs|\
27=www|\
28=aircelgprs|\
29=aircelgprs|\
30=www|\
31=airtelgprs.com|\
33=aircelgprs|\
34=bsnlnet|\
35=aircelgprs|\
36=rcomnet|\
37=aircelgprs|\
38=bsnlnet|\
40=airtelgprs.com|\
41=aircelgprs|\
42=aircelgprs|\
43=www|\
44=internet|\
45=airtelgprs.com|\
46=www|\
49=airtelgprs.com|\
50=smartnet|\
51=bsnlnet|\
52=smartnet|\
53=bsnlnet|\
54=bsnlnet|\
55=bsnlnet|\
56=internet|\
57=bsnlnet|\
58=bsnlnet|\
59=bsnlnet|\
60=www|\
62=bsnlnet|\
64=bsnlnet|\
66=bsnlnet|\
67=smartnet|\
68=mtnl.net|\
69=mtnl.net|\
70=airtelgprs.com|\
71=bsnlnet|\
72=bsnlnet|\
73=bsnlnet|\
74=bsnlnet|\
75=bsnlnet|\
76=bsnlnet|\
77=bsnlnet|\
78=internet|\
79=bsnlnet|\
80=bsnlnet|\
81=bsnlnet|\
82=internet|\
83=smartnet|\
84=www|\
85=smartnet|\
86=www|\
87=internet|\
88=www|\
89=internet|\
90=airtelgprs.com|\
91=aircelweb|\
92=airtelgprs.com|\
93=airtelgprs.com|\
94=airtelgprs.com|\
95=airtelgprs.com|\
96=airtelgprs.com|\
97=airtelgprs.com|\
98=airtelgprs.com|");

}

/**********************************************************************************************
 * Function Name :void apn_404()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 404
 **********************************************************************************************/
void apn_405(char *buffer)
{

strcpy(buffer,"01=rcomnet|\
03=rcomnet|\
04=rcomnet|\
05=rcomnet|\
06=rcomnet|\
07=rcomnet|\
08=rcomnet|\
09=rcomnet|\
10=rcomnet|\
11=rcomnet|\
12=rcomnet|\
13=rcomnet|\
14=rcomnet|\
15=rcomnet|\
17=rcomnet|\
18=rcomnet|\
19=rcomnet|\
20=rcomnet|\
21=rcomnet|\
22=rcomnet|\
23=rcomnet|\
25=TATA.DOCOMO.INTERNET|\
26=TATA.DOCOMO.INTERNET|\
27=TATA.DOCOMO.INTERNET|\
28=TATA.DOCOMO.INTERNET|\
29=TATA.DOCOMO.INTERNET|\
30=TATA.DOCOMO.INTERNET|\
31=TATA.DOCOMO.INTERNET|\
32=TATA.DOCOMO.INTERNET|\
33=TATA.DOCOMO.INTERNET|\
34=TATA.DOCOMO.INTERNET|\
35=TATA.DOCOMO.INTERNET|\
36=TATA.DOCOMO.INTERNET|\
37=TATA.DOCOMO.INTERNET|\
38=TATA.DOCOMO.INTERNET|\
39=TATA.DOCOMO.INTERNET|\
40=TATA.DOCOMO.INTERNET|\
41=TATA.DOCOMO.INTERNET|\
42=TATA.DOCOMO.INTERNET|\
43=TATA.DOCOMO.INTERNET|\
44=TATA.DOCOMO.INTERNET|\
45=TATA.DOCOMO.INTERNET|\
46=TATA.DOCOMO.INTERNET|\
47=TATA.DOCOMO.INTERNET|\
51=airtelgprs.com|\
52=airtelgprs.com|\
53=airtelgprs.com|\
54=airtelgprs.com|\
55=airtelgprs.com|\
56=airtelgprs.com|\
66=www|\
67=www|\
70=internet|\
750=www|\
751=www|\
752=www|\
753=www|\
754=www|\
755=www|\
756=www|\
799=internet|\
800=aircelgprs|\
801=aircelgprs|\
802=aircelgprs|\
803=aircelgprs|\
804=aircelgprs|\
805=aircelgprs|\
806=aircelgprs|\
807=aircelgprs|\
808=aircelgprs|\
809=aircelgprs|\
810=aircelgprs|\
811=aircelgprs|\
812=aircelgprs|\
813=uninor|\
814=uninor|\
815=uninor|\
816=uninor|\
817=uninor|\
818=uninor|\
819=uninor|\
820=uninor|\
821=uninor|\
822=uninor|\
823=vinternet.com|\
824=vinternet.com|\
825=vinternet.com|\
826=vinternet.com|\
827=vinternet.com|\
828=vinternet.com|\
829=vinternet.com|\
830=vinternet.com|\
831=vinternet.com|\
832=vinternet.com|\
833=vinternet.com|\
834=vinternet.com|\
835=vinternet.com|\
836=vinternet.com|\
837=vinternet.com|\
838=vinternet.com|\
839=vinternet.com|\
840=vinternet.com|\
841=vinternet.com|\
842=vinternet.com|\
843=vinternet.com|\
844=uninor|\
845=internet|\
846=internet|\
847=internet|\
848=internet|\
849=internet|\
850=internet|\
851=internet|\
852=internet|\
853=internet|\
854=www|\
855=www|\
856=www|\
857=www|\
858=www|\
859=www|\
860=www|\
861=www|\
862=www|\
863=www|\
864=www|\
865=www|\
866=www|\
867=www|\
868=www|\
869=www|\
870=www|\
871=www|\
872=www|\
873=www|\
874=www|\
875=uninor|\
876=uninor|\
877=uninor|\
878=uninor|\
879=uninor|\
880=uninor|\
881=gprs.stel.in|\
882=gprs.stel.in|\
883=gprs.stel.in|\
884=gprs.stel.in|\
885=gprs.stel.in|\
886=gprs.stel.in|\
908=internet|\
909=internet|\
910=internet|\
911=internet|\
912=internet|\
913=internet|\
914=internet|\
915=internet|\
916=internet|\
917=internet|\
918=internet|\
919=internet|\
920=internet|\
921=internet|\
922=internet|\
923=internet|\
924=internet|\
925=uninor|\
926=uninor|\
927=uninor|\
928=uninor|\
929=uninor|\
930=internet|\
931=internet|\
932=vinternet.com|");

}
/**********************************************************************************************
 * Function Name :void apn_310()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 310
 **********************************************************************************************/
void apn_310 (char *buffer)
{

strcpy(buffer,"00=VZWINTERNET|\
04=VZWINTERNET|\
20=union.wap.com|\
28=VZWINTERNET|\
30=phone|\
32=VZWINTERNET|\
35=VZWINTERNET|\
40=VZWINTERNET|\
50=VZWINTERNET|\
66=internet|\
80=corrgprs|\
90=isp|\
100=isp.plateau|\
120=sprint|\
130=VZWINTERNET|\
150=ndo|\
160=fast.t-mobile.com|\
170=phone|\
180=internet|\
200=fast.t-mobile.com|\
210=fast.t-mobile.com|\
220=fast.t-mobile.com|\
230=fast.t-mobile.com|\
240=fast.t-mobile.com|\
250=fast.t-mobile.com|\
260=fast.t-mobile.com|\
270=fast.t-mobile.com|\
280=agms|\
300=phone|\
310=fast.t-mobile.com|\
320=isp.cellularoneaz.net|\
330=VZWINTERNET|\
360=VZWINTERNET|\
380=proxy|\
390=mms.celloneet.com|\
410=phone|\
420=wap.gocbw.com|\
430=VZWINTERNET|\
450=internet.vedge.com|\
470=phone|\
480=phone|\
490=fast.t-mobile.com|\
530=fast.t-mobile.com|\
560=dobsoncellularwap|\
570=clearsky|\
580=fast.t-mobile.com,VZWINTERNET|\
590=fast.t-mobile.com|\
600=VZWINTERNET|\
610=internet.epictouch|\
640=fast.t-mobile.com|\
660=fast.t-mobile.com|\
720=media.com|\
750=VZWINTERNET|\
770=i2.iwireless.com|\
800=fast.t-mobile.com|\
820=VZWINTERNET|\
840=isp|\
880=wapdtcw.com|\
900=VZWINTERNET|\
910=wap.firstcellular.com|\
920=VZWINTERNET|\
930=VZWINTERNET|\
960=VZWINTERNET|");

}

/**********************************************************************************************
 * Function Name :void apn_311()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 311
 **********************************************************************************************/
void apn_311(char *buffer)
{

strcpy(buffer,"50=VZWINTERNET|\
70=VZWINTERNET|\
80=PINE|\
140=VZWINTERNET|\
190=isp.cellular1.net|\
210=usccinternet|\
220=usccinternet|\
221=usccinternet|\
222=usccinternet|\
223=usccinternet|\
224=usccinternet|\
225=usccinternet|\
226=usccinternet|\
227=usccinternet|\
228=usccinternet|\
229=usccinternet|\
230=tethering.cs4glte.com|\
310=VZWINTERNET|\
340=VZWINTERNET|\
350=VZWINTERNET|\
370=web.acs,web.gci|\
390=VZWINTERNET|\
410=VZWINTERNET|\
420=VZWINTERNET|\
430=VZWINTERNET|\
440=VZWINTERNET|\
450=VZWINTERNET|\
480=VZWINTERNET|\
490=otasn,cinet.spcs|\
500=wap|\
530=wap.mymobiletxt.com|\
580=usccinternet|\
581=usccinternet|\
582=usccinternet|\
583=usccinternet|\
584=usccinternet|\
585=usccinternet|\
586=usccinternet|\
587=usccinternet|\
588=usccinternet|\
589=usccinternet|\
590=VZWINTERNET|\
600=VZWINTERNET|\
610=VZWINTERNET|\
650=VZWINTERNET|\
670=VZWINTERNET|\
740=akcell.mobi|\
870=otasn|\
910=mymn4g.net|\
920=VZWINTERNET|\
930=internet.syringawireless.com|");

}

/**********************************************************************************************
 * Function Name :void apn_316()
 * Parameters :  void
 * Return : void
 * Description : APN codes from 316
 **********************************************************************************************/
void apn_316(char *buffer)
{
  strcpy(buffer,"10=cinet.spcs|");
}