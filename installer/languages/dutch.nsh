;;
;;  Language strings for the Windows Planner NSIS installer.
;;  Windows Code page: 1252
;;

; Make sure to update the PLANNER_MACRO_LANGUAGEFILE_END macro in
; langmacros.nsh when adding new strings to this file

!insertmacro PLANNER_MACRO_ADD_LANGUAGE "Dutch"

; Startup Checks
!insertmacro PLANNER_MACRO_DEFINE_STRING INSTALLER_IS_RUNNING			"Er is al een installatie actief."
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_IS_RUNNING			"Planner wordt op dit moment uitgevoerd. Sluit Planner af en start de installatie opnieuw."

; License Page
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_LICENSE_BUTTON			"Volgende >"
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_LICENSE_BOTTOM_TEXT		"$(^Name) wordt uitgegeven onder de GPL licentie. Deze licentie wordt hier slechts ter informatie aangeboden. $_CLICK"

; Components Page
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_SECTION_TITLE			"Planner (vereist)"
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_TRANSLATIONS_SECTION_TITLE	"Vertalingen"
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_SECTION_DESCRIPTION		"Planner hoofdbestanden en dlls"
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_TRANSLATIONS_SECTION_DESCRIPTION	"Additionele talen"
!insertmacro PLANNER_MACRO_DEFINE_STRING GTK_PRIVATE_SECTION_TITLE              "GTK+ (alleen voor dit programma)"
!insertmacro PLANNER_MACRO_DEFINE_STRING GTK_PRIVATE_SECTION_DESCRIPTION	"GTK+ runtime-omgeving, geinstalleerd op een plaats waar alleen Planner er gebruik van maakt."
!insertmacro PLANNER_MACRO_DEFINE_STRING GTK_PUBLIC_SECTION_TITLE               "GTK+ (gedeelde installatie)"
!insertmacro PLANNER_MACRO_DEFINE_STRING GTK_PUBLIC_SECTION_DESCRIPTION		"GTK+ runtime-omgeving, geinstalleerd voor het hele systeem en bruikbaar voor andere programma's."

; Installer Finish Page
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_FINISH_VISIT_WEB_SITE		"Bezoek de Planner webpagina"

; Planner Section Prompts and Texts
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_UNINSTALL_DESC			"Planner (alleen verwijderen)"
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_PROMPT_WIPEOUT			"Uw oude Planner map staat op het punt verwijderd te worden. Wilt u verder gaan?$\r$\rLet op: Eventuele niet-standaard plugins die u heeft ge�nstalleerd zullen verwijderd worden.$\rPlanner gebruikersinstellingen worden hierdoor niet be�nvloed."
!insertmacro PLANNER_MACRO_DEFINE_STRING PLANNER_PROMPT_DIR_EXISTS		"De installatiemap die u heeft geselecteerd bestaat al. De inhoud$\rvan deze map zal verwijderd worden. Wilt u verder gaan?"

; Uninstall Section Prompts
!insertmacro PLANNER_MACRO_DEFINE_STRING un.PLANNER_UNINSTALL_ERROR_1		"Het verwijderingsprogramma voor Planner kon geen register-ingangen voor Planner vinden.$\rWaarschijnlijk heeft een andere gebruiker het programma ge�nstalleerd."
!insertmacro PLANNER_MACRO_DEFINE_STRING un.PLANNER_UNINSTALL_ERROR_2		"U mag dit programma niet verwijderen."
