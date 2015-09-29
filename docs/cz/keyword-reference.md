Keyword reference (česky)
==========================

and
---------------

Používá se pro logickou operaci **and** mezi dvěmi výrazy
```
<výraz> and <výraz>
```
Vyhodnocuje se zkráceně. Pokud je výsledkem levého výrazu **false**, vrací celý výraz **false** a pravá strana se vůbec neprovede (celá se vynechá). Pokud je výsledkem levého výrazu jiná hodnota než **false**, vykoná se i pravá strana a výsledkem celého výrazuje výsledek pravé strany.

```
$ true and true
Result: true

$ true and print("ahoj")
ahoj
Result: null

$ false and print("ahoj")
Result: false

```

break
---------------

Ukončí právě prováděnou smyčku. Způsobí také, že výsledkem výrazu pro smyčku bude **null**. Takto lze detekovat,
že smyčka byla přerušena příkazem **break**

catch
---------------

Příkaz se používá v bloku try-catch-end

**catch** `<proměnná> <blok>` **end**

Pokud v bloku try-catch dojde k výjimce nebo k chybě, je text výjimky nebo chyby uložen do `<proměnná>` uvedené ihned za příkazem **catch** a posléze se spustí `<blok>` příkazů až do **end**. Povolená je pouze lokální proměnná.


const
---------------

Výraz uvedený za příkazem **const** se přeloží a okamžitě provede. Výsledek výrazu vložen do právě překládaného kódu jako konstanta. Příkaz **const** je zvláštní tím, že provede výpočet výrazu během prekladu!

**const** `<výraz>`;
**const** `(<blok>)`;

Obsah výrazu za příkazem **const** není nikterak omezený. K dispozici by měla být základní verze Tempe, bez funkcí a klíčových slov dodané customizace a bindingu. Samotný příkaz očekává jednoduchý výraz až do prvního středníku. Toto lze rozšířit za použití závorek, protože středníky uvnitř závorek jsou povolené, je možné v rámci příkazu **const** vykonat i složitější kód.

```
foo=const 1+2;  # ekvivaletní foo=3, výraz se spočítá během překladu
foo=const {
     counter: 1;
     inc: function()
            this.counter = this.counter+1
          end }; # konstrukce objektu,  ktery obsahuje metodu během překladu
      
```

Kód uvnitř příkazu **const** není omezen jen na konstanty, tedy jen čísla a řetězce a jejich složení do výrazů. Je možné i **deklarovat proměnné a funkce**. Je třeba si ale uvědomit, v jakém kontextu uvedené proměnné vznikají a kdy konči jejich platnost.

```
$ const a=10; const a+20
Result: 20

$ const fact=function(x)
    a=1;
    while x>1 do
       a=a*x;
       x=x-1;
    end;
    a
    end; a = const fact(10)
Result: 3628800
```

Pro provádění příkazů v rámci **const** je v překladači vytvořen context, který se nazývá **const-scope**. V rámci toho scope je možné vytvářet proměnné, které však nejsou dostupné při běhu kódu. Naopak, běžně deklarované proměnné není možné používat uvnitř příkazu **const**

`a="Hello";b=const a+" world"`  - chyba, proměnná `a` není v **const** dostupná

`const a="Hello";b= a+" world"`  - chyba, proměnná `a` byla deklarovaná uvnitř **const** a není dosupná v běžném kódu

Příkaz **const** ovšem neznamená, že obsahuje neměnnou hodnotu. Zejména objekty a pole vytvořené za pomocí **const** v programu později měnit, pakliže byly přeneseny do běžné proměnné. Použití **const** pouze způsobí, že objekty budou do kódu vloženy již sestrojené, bez ohledu na to, jak moc náročná byla jejich konstrukce. Toho lze využít pro vytváření rozsáhlých definicí a objektů, které lze provést během překladu a do vlastního kódu vložit jen výsledek.

Příkaz **const** má ještě jednu výhodu. Jeho **const-scope** je platný pouze v aktuálním skriptu, bez ohledu na to, kolikrát je do skriptu vložen jiný skript přes příkaz **import**. Tento scope je plně izolován od ostatních scope jiných skriptů. Důležité je také pořadí definic v něm. Zatímco běžné definice se zakládají tak, jak program prochází kódem, kdy to nemusí být přímá cesta, příkazy **const** se provádí vždy v pořadí od zhora dolu, nezávisle na členění vlastního kódu. Lze si v rámci skriptu založit mnoho proměnných v **const-scope** aniž by to ovlivnilo jiné skripty.

defined
---------------
do
---------------
echo
---------------
else
---------------
elseif
---------------
end
---------------
false
---------------
firstDefined
---------------
foreach 
---------------
function
---------------
getvarname
---------------
if
---------------
import
---------------
inline
---------------

Klíčové slovo rozbalí tělo funkce přímo do kódu. Příkaz očekává výraz, který vede na funkci. Je možné použít proměnnou, ale s tou výsadou, že proměnná musí obsahovat funkci a musí být deklarovaná uvnitř příkazu **const**

```
const test=function () 
               a=a+1 
          end; 
a=10; 
inline test; 
inline test
```
Výsledkem operace bude hodnota 12. Pomocí nástroje dumpcode (součástí tempe-console) lze ověřit, že obsah funkce "test" se v místech, kde je uvedeno klíčové slovo `inline` přímo vložil. Argumenty funkce se ignorují a také se při vkládání kódu nevytváří scope. Ve výsledku je tak inlinováná funkce rychleji zpracována, ale s tím, že musí být napsaná na míru danému místu, kde se používá. Proměnné, které ve funkci použijeme se vytváří v aktuálním scope.

Rekurze není povolena a defacto není možná. Není možné zavolat inline na funkci, která ještě nebyla vytvořena. Je samozřejmě možné zavolat funkci jménem, ale pak jde o běžné voláné a nikoliv o operaci `inline`





isnull
---------------

Testuje výraz na pravé straně, zda je výsledkem null. Pokud ano, vrací true, jinak false. 

loop
---------------

Příkaz vyhodnocuje výraz tak dlouho, dokud není výsledkem false. Příkaz očekává jednoduchý výraz, pokud je potřeba opakovat více příkazů, lze použít závorky, nebo volat funkci a testovat její návratovou hodnotu.

```
loop opakuj_dokud_neni_konec();

loop (
   a=rand(1)[0];
   print(a);
a>0);
```

new
---------------
not
---------------
null
---------------
optional
---------------
or
---------------
repeat
---------------

součást bloku repeat-until

repeat `<blok>` until `<vyraz>`

Příkaz opakuje blok tak dlouho, doku vyraz vraci false. Jakmile výraz vrátí true, opakování se zastaví.

scope
---------------

Blokový příkaz scope-end. Založí nový scope a provede blok mezi scope a end. Proměnné vytvořené nebo přepsané uvnitř scope jsou po opuštění scope smazány, případně nahrazeny původní hodnotou

```
a="foo"
scope
   print(a); # vypíše foo
   a="bar";
   print(a); # vypiše bar
end
print(a); # vypíše foo
```

Příkaz neizoluje úplně. Je třeba si dát pozor zejména na objekty, do kterých lze přes tečkový operátor přistupovat a měnite je. Tečkový operátor opouští vliv příkazu scope. Stejnou vlastnosti disponují pole

```
a=[10,20]
scope
  print(a); # vypíše [10,20]
  a[]=30;
  print(a); # vypíše [10,20,30]
end
print(a); # vypíše [10,20,30]
```

Důvodem je, že operace a[]= není založení nové proměnné, ale upráva obsahu existující proměnné.

template
---------------
then
---------------
throw
---------------
true
---------------
try
---------------
unset
---------------
until
---------------
varname
---------------
while
---------------
with
---------------
