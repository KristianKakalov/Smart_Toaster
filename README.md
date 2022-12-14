# Smart_Toaster :bread:

Най-новият индустриален тостер на компания <ACME>, поддържащ изпичане на 16
филии едновременно, предоставя телеметрични данни. Колегите от съседната
лаборатория разработват приложение "Моят умен тостер", което обработва тези
телеметрични данни и ги показва на потребителя в красив вид, смесен с реклами.

Приложението, обаче, има бъг, който те се мъчат да открият от над седмица.
Проблемът е, че за да се репродуцира този бъг, тостерът трябва да се
включи на студено, да се изчака да загрее и да опече 64 филии. На колегите ви
им е омръзнало от миризмата на препечени филийки, и в отчаянието си ви
призовават да напишете програма на C, която да направи възможно тестването на
тяхното приложение без участието на самия тостер.

Тостерът предоставя два вида данни под формата на следните два вида съобщения:

1. съобщение state (8 байта) -- дава информация за температурата на тостера и
за активните слотове за филийки
    - идентификатор на типа данни (2 байта)
        -- за съобщение state винаги е числото 0x0001
    - активни слотове за филийки (2 байта)
        -- битова маска (0/1) - дали съответният слот съдържа филийка,
           най-младшият бит идентифицира първия слот и т.н.
    - температура (4 байта) -- температура в келвини, умножена по 100

2. съобщение slot_text (16 байта) -- съобщение, съдържащо информация за
определен слот
    - идентификатор на типа данни (2 байта)
        -- за съобщение slot_text винаги е числото 0x0002
    - идентификатор на слот (1 байт)
        -- число между 0 и 15, като 0 е първият слот и т.н.
    - текст (13 байта)
        -- C-style низ до 12 символа, който е допълнен с 0x00-символи, така
        че общата дължина е 13 байта

Вашата програма трябва да може да работи в два режима, спрямо подадените
параметри: <br/>
```
    $ ./main record filename.bin <br/>
    $ ./main replay filename.bin <br/>
```

При подаден първи позиционен параметър "record", програмата чете данни от
stdin и ги записва във файла, подаден като втори аргумент, в избран от вас
вид.

Освен самото съобщение, записаните данни във файла трябва да включват и време
на получаване на съобщението (*).

При подаден първи позиционен параметър "replay", програмата чете данни от
файла, подаден като втори аргумент, и да ги извежда на stdout във вида, в
който са приети по време на "record", спазвайки закъсненията между
съобщенията, спрямо каквито са били по време на "record".

И в двата режима (record/replay), освен основната си функция, програмата
извежда на stderr информация за всяко съобщение в следния вид: <br/>
1. при съобщение state:<br/>
&nbsp;&nbsp;&nbsp;&nbsp;[ВРЕМЕ] <state> temp: ТЕМПЕРАТУРА°C, slots: 1[СЪСТ1] 2[СЪСТ2] ... 16[СЪСТ16] <br/>
2. при съобщение slot_text:<br/>
&nbsp;&nbsp;&nbsp;&nbsp;[ВРЕМЕ] <slot text> slot НОМЕР: ТЕКСТ<br/>
Където:<br/>
* ВРЕМЕ е изминалото време от пускане на `record` до получаване на
  съобщението, в секунди, с 3 знака след десетичната точка
* ТЕМПЕРАТУРА е температурата на тостера в градуси целзий с 2 знака след
  десетичната точка
* СЪСТN е знак "X", ако слот N е активен, и знак " ", ако не е
* НОМЕР е число (номер на слот)
* ТЕКСТ е текст, както е получен в съобщението

За тестване ви е предоставен тостеризатор (генератор на тостери), достъпен
през HTTP на адрес https://rnd.qtrp.org/toaster . За всяка нова връзка,
тостериазаторът генерира тостер и праща телеметрията му като отговор.

Командата `curl -Ns https://rnd.qtrp.org/toaster` е лесен начин да получите
тостерна телеметрия на stdout.

Примерен изход:

$ curl -Ns https://rnd.qtrp.org/toaster | ./main record log<br/>
[0.263] <state> temp: -13.15°C, slots: 1[ ] 2[ ] 3[ ] 4[ ] 5[ ] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[1.202] <state> temp: -1.81°C, slots: 1[ ] 2[ ] 3[ ] 4[ ] 5[ ] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[2.007] <state> temp: 7.36°C, slots: 1[ ] 2[ ] 3[ ] 4[ ] 5[ ] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
...<br/>
[38.117] <state> temp: 305.75°C, slots: 1[X] 2[ ] 3[ ] 4[ ] 5[X] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[38.177] <slot text> slot 1: Aman<br/>
[38.240] <slot text> slot 5: North<br/>
[39.221] <slot text> slot 5: North<br/>
[39.258] <slot text> slot 1: Aman<br/>
[39.325] <state> temp: 313.09°C, slots: 1[X] 2[ ] 3[ ] 4[ ] 5[X] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[39.821] <state> temp: 313.12°C, slots: 1[X] 2[ ] 3[ ] 4[ ] 5[X] 6[ ] 7[ ] 8[ ] 9[X] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[39.911] <slot text> slot 9: Morgoth<br/>
[39.947] <slot text> slot 5: North<br/>
[39.972] <slot text> slot 1: Aman<br/>
[40.255] <state> temp: 323.91°C, slots: 1[X] 2[ ] 3[ ] 4[ ] 5[ ] 6[ ] 7[ ] 8[ ] 9[X] 10[ ] 11[ ] 12[X] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[40.269] <slot text> slot 12: Beren<br/>
[40.278] <slot text> slot 9: Morgoth<br/>
[40.312] <slot text> slot 1: Aman<br/>
[40.724] <slot text> slot 1: Aman<br/>
[40.742] <slot text> slot 12: Beren<br/>
[40.750] <slot text> slot 9: Morgoth<br/><br/>

$ ./main replay log > /dev/null<br/>
[0.263] <state> temp: -13.15°C, slots: 1[ ] 2[ ] 3[ ] 4[ ] 5[ ] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[1.202] <state> temp: -1.81°C, slots: 1[ ] 2[ ] 3[ ] 4[ ] 5[ ] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[2.007] <state> temp: 7.36°C, slots: 1[ ] 2[ ] 3[ ] 4[ ] 5[ ] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
...<br/>
[38.117] <state> temp: 305.75°C, slots: 1[X] 2[ ] 3[ ] 4[ ] 5[X] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[38.177] <slot text> slot 1: Aman<br/>
[38.240] <slot text> slot 5: North<br/>
[39.221] <slot text> slot 5: North<br/>
[39.258] <slot text> slot 1: Aman<br/>
[39.325] <state> temp: 313.09°C, slots: 1[X] 2[ ] 3[ ] 4[ ] 5[X] 6[ ] 7[ ] 8[ ] 9[ ] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[39.821] <state> temp: 313.12°C, slots: 1[X] 2[ ] 3[ ] 4[ ] 5[X] 6[ ] 7[ ] 8[ ] 9[X] 10[ ] 11[ ] 12[ ] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[39.911] <slot text> slot 9: Morgoth<br/>
[39.947] <slot text> slot 5: North<br/>
[39.972] <slot text> slot 1: Aman<br/>
[40.255] <state> temp: 323.91°C, slots: 1[X] 2[ ] 3[ ] 4[ ] 5[ ] 6[ ] 7[ ] 8[ ] 9[X] 10[ ] 11[ ] 12[X] 13[ ] 14[ ] 15[ ] 16[ ]<br/>
[40.269] <slot text> slot 12: Beren<br/>
[40.278] <slot text> slot 9: Morgoth<br/>
[40.312] <slot text> slot 1: Aman<br/>
[40.724] <slot text> slot 1: Aman<br/>
[40.742] <slot text> slot 12: Beren<br/>
[40.750] <slot text> slot 9: Morgoth<br/><br/><br/>


Забележки:
1. стремете се да не акумулирате грешка във времената
2. всички числови типове са unsigned.
3. под "време на получаване на съобщението" (*) имаме предвид "брой
милисекунди от пускането на програмата до приключването на първия
`read()`, който е захапал част от това съобщение".
4. за операциите, свързани с време (взимане на текущо време, закъснения),
вижте time(2), nanosleep(2)
5. За да превърнете от келвини в целзий, извадете 273.15
6. Навън е много студено!
7. Тостерът е little endian. По всяка вероятност вашата машина също е little
endian - за улеснение се възползвайте от този факт. Ако случайно сте на
big endian машина, може вместо нея да използвате astero.openfmi.net.
