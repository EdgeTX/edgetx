/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

 #pragma once

// Bootloader strings - ASCII characters only (no extended ASCII or unicode)

#if defined(PCBPL18)
  #if defined(RADIO_NV14_FAMILY)
    #define BL_SELECT_KEY              "[R TRIM]"
    #define BL_EXIT_KEY                "[L TRIM]"
  #elif defined(RADIO_NB4P)
    #define BL_SELECT_KEY              "[SW1A]"
    #define BL_EXIT_KEY                "[SW1B]"
  #elif defined(RADIO_PL18U)
    #define BL_SELECT_KEY              "[KR Dn]"
    #define BL_EXIT_KEY                "[KR Up]"
  #else
    #define BL_SELECT_KEY              "[TR4 Dn]"
    #define BL_EXIT_KEY                "[TR4 Up]"
  #endif
#elif defined(PCBNV14)
  #define BL_SELECT_KEY                "[R TRIM]"
  #define BL_EXIT_KEY                  "[L TRIM]"
#elif defined(RADIO_ST16)
  #define BL_SELECT_KEY                "[ENT]"
  #define BL_EXIT_KEY                  "[EXIT]"
#else
  #define BL_SELECT_KEY                "[ENT]"
  #define BL_EXIT_KEY                  "[RTN]"
#endif

#if defined(TRANSLATIONS_CZ)

    // Poznamka: nutne pouziti textu bez diakritiky - omezeni velikosti pameti bootloader!
    #define TR_BL_USB_CONNECTED           "USB pripojeno"
    #define TR_BL_USB_PLUGIN              "nebo pripojte USB kabel"
    #define TR_BL_USB_MASS_STORE          "pro pouziti uloziste"
    #define TR_BL_WRITE_FW                "Nahrat firmware"
    #define TR_BL_FORK                    "Fork:"
    #define TR_BL_VERSION                 "Verze:"
    #define TR_BL_RADIO                   "Radio:"
    #define TR_BL_EXIT                    "Ukoncit"
    #define TR_BL_DIR_MISSING             "Adresar chybi"
    #define TR_BL_DIR_EMPTY               "Adresar je prazdny"
    #define TR_BL_WRITING_FW              "Nahravani firmware ..."
    #define TR_BL_WRITING_COMPL           "Nahravani dokonceno"
    #define TR_BL_ENABLE                  "Povoleno"
    #define TR_BL_DISABLE                 "Zakazano"

    #if defined(COLORLCD)
        #define TR_BL_INVALID_FIRMWARE       "Neplatny soubor s firmwarem"
    #elif LCD_W >= 212
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN " " TR_BL_USB_MASS_STORE
        #define TR_BL_HOLD_ENTER_TO_START    "Drzet [ENT] pro zahajeni nahravani"
        #define TR_BL_INVALID_FIRMWARE       "Neplatny firmware soubor!"
    #else
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
        #define TR_BL_HOLD_ENTER_TO_START    "Drzet [ENT] pro zahajeni"
        #define TR_BL_INVALID_FIRMWARE       "Neplatny firmware!"
    #endif

    // Bootloader Taranis specific - ASCII characters only
    #define TR_BL_POWER_KEY                "Stisknete tlacitko napajeni."
    #define TR_BL_FLASH_EXIT               "Ukoncit rezim nahravani."

    // Bootloader Horus specific - ASCII characters only
    #define TR_BL_ERASE_INT_FLASH          "Erase Internal Flash Storage"
    #define TR_BL_ERASE_FLASH              "Erase Flash Storage"
    #define TR_BL_ERASE_FLASH_MSG          "This may take up to 200s"
    #define TR_BL_RF_USB_ACCESS            "RF USB access"
    #define TR_BL_CURRENT_FW               "Aktualni firmware:"

    #define TR_BL_SELECT_KEY               BL_SELECT_KEY " pro vybrani souboru"
    #define TR_BL_FLASH_KEY                "Drzet dlouze " BL_SELECT_KEY " pro nahrani"
    #define TR_BL_ERASE_KEY                "Hold " BL_SELECT_KEY " long to erase"
    #define TR_BL_TOGGLE_KEY               BL_SELECT_KEY " to toggle"
    #define TR_BL_EXIT_KEY                 BL_EXIT_KEY " pro ukonceni"

#elif defined(TRANSLATIONS_DA)

    #define TR_BL_USB_CONNECTED           "USB forbundet"
    #define TR_BL_USB_PLUGIN              "eller brug USB kabel"
    #define TR_BL_USB_MASS_STORE          "for USB disk"
    #define TR_BL_WRITE_FW                "Installer firmware"
    #define TR_BL_FORK                    "Fork:"
    #define TR_BL_VERSION                 "Version:"
    #define TR_BL_RADIO                   "Radio:"
    #define TR_BL_EXIT                    "Forlad"
    #define TR_BL_DIR_MISSING             "Katalog mangler"
    #define TR_BL_DIR_EMPTY               "Katalog er tomt"
    #define TR_BL_WRITING_FW              "Installerer..."
    #define TR_BL_WRITING_COMPL           "Installation slut"
    #define TR_BL_ENABLE                  "Aktiver"
    #define TR_BL_DISABLE                 "Deaktiver"

    #if defined(COLORLCD)
        #define TR_BL_INVALID_FIRMWARE       "Ikke en installationsfil!"
    #elif LCD_W >= 212
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN " " TR_BL_USB_MASS_STORE
        #define TR_BL_HOLD_ENTER_TO_START    "[ENT] for at starte installation"
        #define TR_BL_INVALID_FIRMWARE       "Ikke en installationsfil!"
    #else
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
        #define TR_BL_HOLD_ENTER_TO_START    "[ENT] for at starte"
        #define TR_BL_INVALID_FIRMWARE       "Ikke en installationsfil!"
    #endif

    // Bootloader Taranis specific - ASCII characters only
    #define TR_BL_POWER_KEY                "Tryk power knap."
    #define TR_BL_FLASH_EXIT               "Forlad installation tilstand."

    // Bootloader Horus specific - ASCII characters only
    #define TR_BL_ERASE_INT_FLASH          "Slet intern flash lager"
    #define TR_BL_ERASE_FLASH              "Slet flash lager"
    #define TR_BL_ERASE_FLASH_MSG          "Dette kan vare op til 200s"
    #define TR_BL_RF_USB_ACCESS            "RF USB adgang"
    #define TR_BL_CURRENT_FW               "Current firmware:"

    #define TR_BL_SELECT_KEY               BL_SELECT_KEY " for at bruge fil"
    #define TR_BL_FLASH_KEY                "Hold " BL_SELECT_KEY " laenge, for at starte"
    #define TR_BL_ERASE_KEY                "Hold " BL_SELECT_KEY " laenge, for at slette"
    #define TR_BL_TOGGLE_KEY               BL_SELECT_KEY " to toggle"
    #define TR_BL_EXIT_KEY                 BL_EXIT_KEY " for at forlade"

#elif defined(TRANSLATIONS_DE)

    #define TR_BL_USB_CONNECTED           "USB verbunden"
    #define TR_BL_USB_PLUGIN              "oder USB-Kabel anschl."
    #define TR_BL_USB_MASS_STORE          "für den SD-Speicher an"
    #define TR_BL_WRITE_FW                "Schreibe Firmware"
    #define TR_BL_FORK                    "Fork:"
    #define TR_BL_VERSION                 "Version:"
    #define TR_BL_RADIO                   "Radio:"
    #define TR_BL_EXIT                    "Beenden"
    #define TR_BL_DIR_MISSING             "Verzeichnis fehlt"
    #define TR_BL_DIR_EMPTY               "Verzeichnis leer"
    #define TR_BL_WRITING_FW              "Schreibe..."
    #define TR_BL_WRITING_COMPL           TR("Schreiben fertig","Schreiben abgeschlossen")
    #define TR_BL_ENABLE                  "Aktivieren"
    #define TR_BL_DISABLE                 "Deaktivieren"

    #if defined(COLORLCD)
        #define TR_BL_INVALID_FIRMWARE       "Keine gültige Firmwaredatei"
    #elif LCD_W >= 212
        #define TR_BL_OR_PLUGIN_USB_CABLE    "Oder schließen Sie ein USB-Kabel " TR_BL_USB_MASS_STORE
        #define TR_BL_HOLD_ENTER_TO_START    "Halte [ENT] gedrückt, um mit dem Schreiben zu beginnen"
        #define TR_BL_INVALID_FIRMWARE       "Keine gültige Firmwaredatei!"
    #else
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
        #define TR_BL_HOLD_ENTER_TO_START    "Zum Starten [ENT] halten"
        #define TR_BL_INVALID_FIRMWARE       "Keine gültige Firmware!"
    #endif

    // Bootloader Taranis specific - ASCII characters only
    #define TR_BL_POWER_KEY                "Drücke den Power Knopf."
    #define TR_BL_FLASH_EXIT               "Verlasse den Flashmodus."

    // Bootloader Horus specific - ASCII characters only
    #define TR_BL_ERASE_INT_FLASH          "Erase Internal Flash Storage"
    #define TR_BL_ERASE_FLASH              "Erase Flash Storage"
    #define TR_BL_ERASE_FLASH_MSG          "This may take up to 200s"
    #define TR_BL_RF_USB_ACCESS            "RF USB Zugriff"
    #define TR_BL_CURRENT_FW               "Aktuelle Firmware:"

    #define TR_BL_SELECT_KEY               BL_SELECT_KEY " um Datei auszuwählen"
    #define TR_BL_FLASH_KEY                "Halte " BL_SELECT_KEY " gedrückt, zum schreiben"
    #define TR_BL_ERASE_KEY                "Halte " BL_SELECT_KEY " gedrückt, to erase"
    #define TR_BL_TOGGLE_KEY               BL_SELECT_KEY " to toggle"
    #define TR_BL_EXIT_KEY                 BL_EXIT_KEY " zum beenden"

#elif defined(TRANSLATIONS_FR)

    #define TR_BL_USB_CONNECTED           "USB Connecte"
    #define TR_BL_USB_PLUGIN              "ou branchez cable USB"
    #define TR_BL_USB_MASS_STORE          "pour stockage de masse"
    #define TR_BL_WRITE_FW                "Ecriture Firmware"
    #define TR_BL_FORK                    "Fork:"
    #define TR_BL_VERSION                 "Version:"
    #define TR_BL_RADIO                   "Radio:"
    #define TR_BL_EXIT                    "Quitter"
    #define TR_BL_DIR_MISSING             "Repertoire absent"
    #define TR_BL_DIR_EMPTY               "Repertoire vide"
    #define TR_BL_WRITING_FW              "Ecriture Firmware ..."
    #define TR_BL_WRITING_COMPL           "Ecriture terminée"
    #define TR_BL_ENABLE                  "Activer"
    #define TR_BL_DISABLE                 "Désactiver"

    #if defined(COLORLCD)
        #define TR_BL_INVALID_FIRMWARE       "Fichier firmware non valide"
    #elif LCD_W >= 212
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN " " TR_BL_USB_MASS_STORE
        #define TR_BL_HOLD_ENTER_TO_START    "Appui [ENT] pour demarrer ecriture"
        #define TR_BL_INVALID_FIRMWARE       "Fichier firmware non valide!"
    #else
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
        #define TR_BL_HOLD_ENTER_TO_START    "Appui [ENT] pour demarrer"
        #define TR_BL_INVALID_FIRMWARE       "Firmware non valide!"
    #endif

    // Bootloader Taranis specific - ASCII characters only
    #define TR_BL_POWER_KEY                "Appuyez sur le bouton power."
    #define TR_BL_FLASH_EXIT               "Quitter mode flashage."

    // Bootloader Horus specific - ASCII characters only
    #define TR_BL_ERASE_INT_FLASH          "Erase Internal Flash Storage"
    #define TR_BL_ERASE_FLASH              "Erase Flash Storage"
    #define TR_BL_ERASE_FLASH_MSG          "This may take up to 200s"
    #define TR_BL_RF_USB_ACCESS            "RF USB access"
    #define TR_BL_CURRENT_FW               "Firmware actuel:"

    #define TR_BL_SELECT_KEY               BL_SELECT_KEY " pour select. fichier"
    #define TR_BL_FLASH_KEY                "Appui long " BL_SELECT_KEY " pour flasher"
    #define TR_BL_ERASE_KEY                "Appui long " BL_SELECT_KEY " to erase"
    #define TR_BL_TOGGLE_KEY               BL_SELECT_KEY " to toggle"
    #define TR_BL_EXIT_KEY                 BL_EXIT_KEY " pour quitter"

#elif defined(TRANSLATIONS_IT)

    #define TR_BL_USB_CONNECTED           "USB Connessa"
    #define TR_BL_USB_PLUGIN              "O connetti il cavo USB"
    #define TR_BL_USB_MASS_STORE          "per memoria di massa"
    #define TR_BL_WRITE_FW                "Scrivo il Firmware"
    #define TR_BL_FORK                    "Fork:"
    #define TR_BL_VERSION                 "Versione:"
    #define TR_BL_RADIO                   "Radio:"
    #define TR_BL_EXIT                    "Esci"
    #define TR_BL_DIR_MISSING             "Cartella non trovata"
    #define TR_BL_DIR_EMPTY               "Cartella vuota"
    #define TR_BL_WRITING_FW              "Scrittura Firmware..."
    #define TR_BL_WRITING_COMPL           "Scrittura completata"
    #define TR_BL_ENABLE                  "Abilita"
    #define TR_BL_DISABLE                 "Disabilita"

    #if defined(COLORLCD)
        #define TR_BL_INVALID_FIRMWARE       "Firmware non valido"
    #elif LCD_W >= 212
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN " " TR_BL_USB_MASS_STORE
        #define TR_BL_HOLD_ENTER_TO_START    "Premi [ENT] per iniziare la scrittura"
        #define TR_BL_INVALID_FIRMWARE       "Non è un file Firmware valido!"
    #else
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
        #define TR_BL_HOLD_ENTER_TO_START    "Premi [ENT] per partire"
        #define TR_BL_INVALID_FIRMWARE       "Non è un file Firmware valido!"
    #endif

    // Bootloader Taranis specific - ASCII characters only
    #define TR_BL_POWER_KEY                "Premi il bottone d'accensione"
    #define TR_BL_FLASH_EXIT               "Esci dal modo scrittura."

    // Bootloader Horus specific - ASCII characters only
    #define TR_BL_ERASE_INT_FLASH          "Erase Internal Flash Storage"
    #define TR_BL_ERASE_FLASH              "Erase Flash Storage"
    #define TR_BL_ERASE_FLASH_MSG          "This may take up to 200s"
    #define TR_BL_RF_USB_ACCESS            "RF USB access"
    #define TR_BL_CURRENT_FW               "Current firmware:"

    #define TR_BL_SELECT_KEY               BL_SELECT_KEY " per scegliere il file"
    #define TR_BL_FLASH_KEY                "Tenere premuto " BL_SELECT_KEY " per scrivere"
    #define TR_BL_ERASE_KEY                "Hold " BL_SELECT_KEY " long to erase"
    #define TR_BL_TOGGLE_KEY               BL_SELECT_KEY " to toggle"
    #define TR_BL_EXIT_KEY                 BL_EXIT_KEY " per uscire"

#elif defined(TRANSLATIONS_PL)

    #define TR_BL_USB_CONNECTED           "USB polaczone"
    #define TR_BL_USB_PLUGIN              "lub podlacz kabel USB"
    #define TR_BL_USB_MASS_STORE          "dla trybu danych"
    #define TR_BL_WRITE_FW                "Zapis firmware"
    #define TR_BL_FORK                    "Fork:"
    #define TR_BL_VERSION                 "Wersja:"
    #define TR_BL_RADIO                   "Radio:"
    #define TR_BL_EXIT                    "Wyjdz"
    #define TR_BL_DIR_MISSING             "Brak katalogu"
    #define TR_BL_DIR_EMPTY               "Katalog jest pusty"
    #define TR_BL_WRITING_FW              "Zapis firmware ..."
    #define TR_BL_WRITING_COMPL           "Zapis ukonczony"
    #define TR_BL_ENABLE                  "Enable"
    #define TR_BL_DISABLE                 "Disable"

    #if defined(COLORLCD)
        #define TR_BL_INVALID_FIRMWARE       "Nieprawidlowy plik firmware"
    #elif LCD_W >= 212
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN " " TR_BL_USB_MASS_STORE
        #define TR_BL_HOLD_ENTER_TO_START    "Przytrzymaj [ENT] by zaczac zapis"
        #define TR_BL_INVALID_FIRMWARE       "Nieprawidlowy plik firmware!"
    #else
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
        #define TR_BL_HOLD_ENTER_TO_START    "Przytrzymaj [ENT] by zaczac"
        #define TR_BL_INVALID_FIRMWARE       "011Nieprawidlowy firmware!"
    #endif

    // Bootloader Taranis specific - ASCII characters only
    #define TR_BL_POWER_KEY                "Nacisnij przycisk Power"
    #define TR_BL_FLASH_EXIT               "Wyjdz z trybu flashowania"

    // Bootloader Horus specific - ASCII characters only
    #define TR_BL_ERASE_INT_FLASH          "Erase Internal Flash Storage"
    #define TR_BL_ERASE_FLASH              "Erase Flash Storage"
    #define TR_BL_ERASE_FLASH_MSG          "This may take up to 200s"
    #define TR_BL_RF_USB_ACCESS            "RF USB access"
    #define TR_BL_CURRENT_FW               "Obecny firmware:"

    #define TR_BL_SELECT_KEY               BL_SELECT_KEY " aby wybrac plik"
    #define TR_BL_FLASH_KEY                "Przytrzymaj " BL_SELECT_KEY " aby flashowac"
    #define TR_BL_ERASE_KEY                "Przytrzymaj " BL_SELECT_KEY " long to erase"
    #define TR_BL_TOGGLE_KEY               BL_SELECT_KEY " to toggle"
    #define TR_BL_EXIT_KEY                 BL_EXIT_KEY " aby wyjsc"

#elif defined(TRANSLATIONS_SE)

    // Bootloader common
    #define TR_BL_USB_CONNECTED            "USB ansluten"
    #define TR_BL_USB_PLUGIN               "Eller anslut med USB-kabel"
    #define TR_BL_USB_MASS_STORE           "foer masslagring"
    #define TR_BL_WRITE_FW                 "Skriv firmware"
    #define TR_BL_FORK                     "Gren:"
    #define TR_BL_VERSION                  "Version:"
    #define TR_BL_RADIO                    "Radio:"
    #define TR_BL_EXIT                     "Avsluta"
    #define TR_BL_DIR_MISSING              "Katalogen saknas"
    #define TR_BL_DIR_EMPTY                "Katalogen aer tom"
    #define TR_BL_WRITING_FW               "Skriver..."
    #define TR_BL_WRITING_COMPL            "Skrivning klar"
    #define TR_BL_ENABLE                   "Aktivera"
    #define TR_BL_DISABLE                  "Inaktivera"

    #if LCD_W >= 480
        #define TR_BL_INVALID_FIRMWARE        "Ej en giltig firmwarefil"
    #elif LCD_W >= 212
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN " " TR_BL_USB_MASS_STORE
        #define TR_BL_HOLD_ENTER_TO_START     "Tryck [ENT] foer att boerja skriva  "
        #define TR_BL_INVALID_FIRMWARE        "Ej en giltig firmwarefil!"
    #else
        #define TR_BL_OR_PLUGIN_USB_CABLE     TR_BL_USB_PLUGIN
        #define TR_BL_HOLD_ENTER_TO_START     "Tryck [ENT] för att boerja"
        #define TR_BL_INVALID_FIRMWARE        "Ej en giltig firmware!"
    #endif

    // Bootloader Taranis specific - ASCII characters only
    #define TR_BL_POWER_KEY                 "Tryck paa startknappen."
    #define TR_BL_FLASH_EXIT                "Avsluta flashningslaeget."

    // Bootloader Horus specific - ASCII characters only
    #define TR_BL_ERASE_INT_FLASH           "Radera interna flashminnet"
    #define TR_BL_ERASE_FLASH               "Radera flashminnet"
    #define TR_BL_ERASE_FLASH_MSG           "Detta kan ta upp till 200s"
    #define TR_BL_RF_USB_ACCESS             "RF USB-access"
    #define TR_BL_CURRENT_FW                "Nuvarande firmware:"

    #define TR_BL_SELECT_KEY               BL_SELECT_KEY " foer att vaelja fil"
    #define TR_BL_FLASH_KEY                "Haall ner " BL_SELECT_KEY " foer att flasha"
    #define TR_BL_ERASE_KEY                "Haall ner " BL_SELECT_KEY " foer att radera"
    #define TR_BL_TOGGLE_KEY               BL_SELECT_KEY " to toggle"
    #define TR_BL_EXIT_KEY                 BL_EXIT_KEY " foer att avbryta"

#else

    #define TR_BL_USB_CONNECTED           "USB Connected"
    #define TR_BL_USB_PLUGIN              "Or plug in a USB cable"
    #define TR_BL_USB_MASS_STORE          "for mass storage"
    #define TR_BL_WRITE_FW                "Write Firmware"
    #define TR_BL_FORK                    "Fork:"
    #define TR_BL_VERSION                 "Version:"
    #define TR_BL_RADIO                   "Radio:"
    #define TR_BL_EXIT                    "Exit"
    #define TR_BL_DIR_MISSING             "Directory is missing"
    #define TR_BL_DIR_EMPTY               "Directory is empty"
    #define TR_BL_WRITING_FW              "Writing..."
    #define TR_BL_WRITING_COMPL           "Writing complete"
    #define TR_BL_ENABLE                  "Enable"
    #define TR_BL_DISABLE                 "Disable"

    #if defined(COLORLCD)
        #define TR_BL_INVALID_FIRMWARE       "Not a valid firmware file"
    #elif LCD_W >= 212
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN " " TR_BL_USB_MASS_STORE
        #define TR_BL_HOLD_ENTER_TO_START    "Hold [ENT] to start writing"
        #define TR_BL_INVALID_FIRMWARE       "Not a valid firmware file!"
    #else
        #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
        #define TR_BL_HOLD_ENTER_TO_START    "Hold [ENT] to start"
        #define TR_BL_INVALID_FIRMWARE       "Not a valid firmware!"
    #endif

    // Bootloader Taranis specific - ASCII characters only
    #define TR_BL_POWER_KEY                "Press the power button."
    #define TR_BL_FLASH_EXIT               "Exit the flashing mode."

    // Bootloader Horus specific - ASCII characters only
    #define TR_BL_ERASE_INT_FLASH          "Erase Internal Flash Storage"
    #define TR_BL_ERASE_FLASH              "Erase Flash Storage"
    #define TR_BL_ERASE_FLASH_MSG          "This may take up to 200s"
    #define TR_BL_RF_USB_ACCESS            "RF USB access"
    #define TR_BL_CURRENT_FW               "Current Firmware:"

    #define TR_BL_SELECT_KEY               BL_SELECT_KEY " to select file"
    #define TR_BL_FLASH_KEY                "Hold " BL_SELECT_KEY " long to flash"
    #define TR_BL_ERASE_KEY                "Hold " BL_SELECT_KEY " long to erase"
    #define TR_BL_TOGGLE_KEY               BL_SELECT_KEY " to toggle"
    #define TR_BL_EXIT_KEY                 BL_EXIT_KEY " to exit"

#endif

#define TR_BL_DFU_MODE          "DFU mode"
#define TR_BL_PLUG_USB          "Plug USB cable"
#define TR_BL_COPY_UF2          "Copy firmware.uf2 to EDGETX_UF2 drive"
#define TR_BL_NO_VERSION        "No version"