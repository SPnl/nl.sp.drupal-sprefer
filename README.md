Deze module maakt het mogelijk blokken met aanmeldformulieren te genereren waarbij mensen die deze invullen een email met daarin een persoonlijke link toegestuurd krijgen. In deze link wordt het CiviCRM contact id van degene die het formulier heeft ingevuld gezet middels een token. De bedoeling is dat deze link verwijst naar een webformulier, waarbij het contact id als waarde in de url wordt meegegeven. De url in de instellingen van de e-mail inhoud ziet er dus ongeveer zo uit: https://doemee.sp.nl/ik_doe_mee_formulier?cid=[contact_id].

In de SP Refer instellingen moeten de titel van het blok, het onderwerp en inhoud van de e-mail, en inhoud van de feedback pagina (waar je aangeeft dat iemand een mail gaat ontvangen met een link erin, en wat met deze link te doen e.d.) worden ingevuld.

Het is nu de bedoeling dat in het webformulier waar de link naar verwijst een verborgen veld wordt toegevoegd. De default waarde van dit verborgen veld moet nu middels een token gevuld worden: [current-page:query:cid]. Op deze manier wordt het contact id van degene van wie iemand de link naar het formulier heeft gekregen opgeslagen in het formulier.

Nu kan er in de instellingen van de webformuliersynchronizatie een sprefer veld worden aangegeven. Kies hiervoor het betreffende verborgen veld. Nu moet ook worden aangegeven in welke CiviCRM groep het contact behorende bij de id in dat veld moet worden gezet.

Als nu iemand op de link klikt die deze van iemand heeft gekregen, en het formulier invult, dan wordt degene van wie hij of zij de link heeft gekregen in de ingestelde groep toegevoegd. Daarnaast is de webformsync zo als anders in te stellen zodat ook de gegevens van de persoon die het webformulier invult in CiviCRM terecht komen.
