int readAnalog(int pin)
{
  digitalWrite(S0, bitRead(pin, 0));
  digitalWrite(S1, bitRead(pin, 1));
  digitalWrite(S2, bitRead(pin, 2));
  /*
  Debug.println();
  Debug.print(bitRead(pin, 2));
  Debug.print(bitRead(pin, 1));
  Debug.print(bitRead(pin, 0));
  Debug.println();
  Debug.println(analogRead(A0));
  */
  return analogRead(A0);
}

time_t sleepingTime()
{
  return now()+timeGoingToSleep;
}


void digitalClockDisplay()
{
  // digital clock display of the time
  Debug.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.println(year());
  Debug.println();
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Debug.print(":");
  if (digits < 10)
    Debug.print('0');
  Debug.print(digits);
}

int readTime()
{
  time_t ntpTime = getNtpTime();
  if(ntpTime == 0)
  {
    ntpStatus = false;
    digitalClockDisplay();
    return 0;
  }
  else
  {
    ntpStatus = true;
    setTime(ntpTime);
    digitalClockDisplay();
    return 1;
  }
}

void convertOpenCloseTime()
{
  TimeElements tmOpen;
  breakTime(now(), tmOpen);
  tmOpen.Minute = openingTimeMinute;
  tmOpen.Hour = openingTimeHour;
  openTime = makeTime(tmOpen);

  TimeElements tmClose;
  breakTime(now(), tmClose);
  tmClose.Minute = closingTimeMinute;
  tmClose.Hour = closingTimeHour;
  closeTime = makeTime(tmClose);
}

void calculerLeverCoucherSoleil()
{
  calculerEphemeride(day(), month(), year(), longitude, latitude, &leverSoleil, &meridien, &coucherSoleil);
  calculerHeureOpen(leverSoleil);
  calculerHeureClose(coucherSoleil);
  convertOpenCloseTime();
  openingTimeHour+=openAfterSunrise;
  closingTimeHour+=closeAfterSunset;
  //send to HTML SERVER
  String openingTime = String(openingTimeHour)+"h"+String(openingTimeMinute);
  String closingTime = String(closingTimeHour)+"h"+String(closingTimeMinute);
  httpEvents.send(openingTime.c_str(), "HEURE OUVERTURE");
  httpEvents.send(closingTime.c_str(), "HEURE FERMETURE");
  Debug.println("Heure d'ouverture : "+String(openingTimeHour)+"h"+String(openingTimeMinute));
  Debug.println("Heure de fermeture : "+String(closingTimeHour)+"h"+String(closingTimeMinute));
}

void calculerEphemeride(int jour, int mois, int annee, double longitude_ouest, double latitude_nord, double *lever, double *meridien, double *coucher)
{
  int nbjours;
  const double radians = M_PI / 180.0;
  double d, x, sinlat, coslat;

  //calcul nb jours ÈcoulÈs depuis le 01/01/2000
  if (annee > 2000) annee -= 2000;
  nbjours = (annee*365) + ((annee+3)>>2) + jour - 1;
  switch (mois)
  {
    case  2 : nbjours +=  31; break;
    case  3 : nbjours +=  59; break;
    case  4 : nbjours +=  90; break;
    case  5 : nbjours += 120; break;
    case  6 : nbjours += 151; break;
    case  7 : nbjours += 181; break;
    case  8 : nbjours += 212; break;
    case  9 : nbjours += 243; break;
    case 10 : nbjours += 273; break;
    case 11 : nbjours += 304; break;
    case 12 : nbjours += 334; break;
  }
  if ((mois > 2) && (annee&3 == 0)) nbjours++;
  d = nbjours;

  //calcul initial meridien & lever & coucher
  x = radians * latitude_nord;
  sinlat = sin(x);
  coslat = cos(x);
  calculerCentreEtVariation(longitude_ouest, sinlat, coslat, d + longitude_ouest/360.0, meridien, &x);
  *lever = *meridien - x;
  *coucher = *meridien + x;

  //seconde itÈration pour une meilleure prÈcision de calcul du lever
  calculerCentreEtVariation(longitude_ouest, sinlat, coslat, d + *lever, lever, &x);
  *lever = *lever - x;

  //seconde itÈration pour une meilleure prÈcision de calcul du coucher
  calculerCentreEtVariation(longitude_ouest, sinlat, coslat, d + *coucher, coucher, &x);
  *coucher = *coucher + x;
}

void calculerCentreEtVariation(double longitude_ouest, double sinlat, double coslat, double d, double *centre, double *variation)
{
  //constantes prÈcalculÈes par le compilateur
  const double M_2PI = 2.0 * M_PI;
  const double degres = 180.0 / M_PI;
  const double radians = M_PI / 180.0;
  const double radians2 = M_PI / 90.0;
  const double m0 = 357.5291;
  const double m1 = 0.98560028;
  const double l0 = 280.4665;
  const double l1 = 0.98564736;
  const double c0 = 0.01671;
  const double c1 = degres * (2.0*c0 - c0*c0*c0/4.0);
  const double c2 = degres * c0*c0 * 5.0 / 4.0;
  const double c3 = degres * c0*c0*c0 * 13.0 / 12.0;
  const double r1 = 0.207447644182976; // = tan(23.43929 / 180.0 * M_PI / 2.0)
  const double r2 = r1*r1;
  const double d0 = 0.397777138139599; // = sin(23.43929 / 180.0 * M_PI)
  const double o0 = -0.0106463073113138; // = sin(-36.6 / 60.0 * M_PI / 180.0)

  double M,C,L,R,dec,omega,x;

  //deux ou trois petites formules de calcul
  M = radians * fmod(m0 + m1 * d, 360.0);
  C = c1*sin(M) + c2*sin(2.0*M) + c3*sin(3.0*M);
  L = fmod(l0 + l1 * d + C, 360.0);
  x = radians2 * L;
  R = -degres * atan((r2*sin(x))/(1+r2*cos(x)));
  *centre = (C + R + longitude_ouest)/360.0;

  dec = asin(d0*sin(radians*L));
  omega = (o0 - sin(dec)*sinlat)/(cos(dec)*coslat);
  if ((omega > -1.0) && (omega < 1.0))
    *variation = acos(omega) / M_2PI;
  else
    *variation = 0.0;
}

void calculerHeureOpen(double d)
{
  int h,m,s;

  d = d + 0.5;

  if (d < 0.0)
  {
    d = d + 1.0;
  }
  else
  {
    if (d > 1.0)
    {
      d = d - 1.0;
    }
  }

  openingTimeHour = d * 24.0;
  d = d - (double) h / 24.0;
  m = d * 1440.0;
  String minutesLong = String(m);
  String minutes = minutesLong.substring(0, 2);
  openingTimeMinute = minutes.toInt();
  openingTimeHour+=timeZone;
}

void calculerHeureClose(double d)
{
  int h,m,s;

  d = d + 0.5;

  if (d < 0.0)
  {
    d = d + 1.0;
  }
  else
  {
    if (d > 1.0)
    {
      d = d - 1.0;
    }
  }

  closingTimeHour = d * 24.0;
  d = d - (double) h / 24.0;
  m = d * 1440.0;
  String minutesLong = String(m);
  String minutes = minutesLong.substring(0, 2);
  closingTimeMinute = minutes.toInt();
  closingTimeHour+=timeZone;
}
  
