/*
*langFrench.cpp : Strings for French language.
*
*	*** Warning ***
* This file is saved in UTF-8 with signature. If you remove this encoding, the strings will no more be
* as expected on screen. Watchout, some editor will remove them without notice. Visual Studio preserves
* them. You can restore them with Visual Studio by doing save as and in the save button, select save
* with encoding.
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/

#include <stdafx.h>
#include "lang.h"

// Forward declaration to force the defintion to fit with the declaration in lang.cpp
extern const LangMsg langFrench[];

const LangMsg langFrench[] =
{	// The order in this list is not important.
	{ TXT_DEVLIST,				_T("Liste des appareils AMI") },
	{ TXT_PRODUCT,				_T("Produit") },
	{ TXT_SERNUM,				_T("Numéro de série") },
	{ TXT_CURFWVER,				_T("Version actuelle de l'application") },
	{ TXT_NEWFWVER,				_T("Version à mettre à jour") },
	{ TXT_UPGRKEY,				_T("Clé d'amélioration") },
	{ TXT_REFRESH_BTN,			_T("Rafraîchir") },
	{ TXT_UPDATE_BTN,			_T("Mettre à jour") },
	{ TXT_UPGRADE_BTN,			_T("Améliorer") },
	{ TXT_UPDATE_INFO,			_T("Début de la mise à jour. Peut prendre quelques secondes avant de commencer.") },
	{ TXT_EXIT_BTN,				_T("Quitter") },
	{ TXT_ERR_DEV_SHORTMSG      _T("L'unité a détecté un message trop court") },
	{ TXT_ERR_DEV_INVCMD        _T("L'unité a détecté une commande invalide") },
	{ TXT_ERR_DEV_ERASING,		_T("Erreur d'effacement dans l'unité") },
	{ TXT_ERR_DEV_PROGRAMMING,	_T("Erreur de programmation dans l'unité") },
	{ TXT_ERR_DEV_READING,		_T("Erreur de lecture mémoire dans l'unité") },
	{ TXT_ERR_CRC_DWLD,			_T("Erreur d'intégrité lors du transfert") },
	{ TXT_ERR_CRC_SAVE,			_T("Erreur d'intégrité lors de la programmation") },
	{ TXT_ERR_INCOMPATIBLE,		_T("Application incompatible") },
	{ TXT_ERR_DEVICE,			_T("Erreur appareil") },
	{ TXT_ERR_DONE,				_T("Mise à jour terminée") },
	{ TXT_ERR_SENDFAIL,			_T("L'envoi a échoué") },
	{ TXT_ERR_CONNECTFAIL,		_T("Erreur lors de la connexion") },
    { TXT_ERR_UNREACHABLE,      _T("Unité inaccessible") },
	{ TXT_ERR_NOANSWER,			_T("Aucune réponse") },
	{ TXT_ERR_ABORTED,			_T("Transfert interrompu") },
	{ TXT_ERR_RXFAIL,			_T("Réception échouée") },
	{ TXT_ERR_INV_MSGTYPE,		_T("Réception d'un message invalide") },
	{ TXT_ERR_INV_RESPLEN,		_T("Réception d'une longueur de réponse invalide") },
	{ TXT_ERR_INV_KEY,			_T("Clé invalide") },
	{ TXT_ERR_INFO_GATHER,		_T("Erreur collecte d'information") },
	{ NULL, NULL }				// end of list
};
