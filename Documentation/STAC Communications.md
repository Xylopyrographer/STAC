# STAC Communications

## Introduction

Short techie document describing the way a STAC communicates with a Roland model V-60HD video switch using the Smart Tally protocol.

This is only relevant if you are using a STAC in an emulated environment. That is, where the STAC is not communicating with a physical Roland device but is operating in a system that emulates a Roland device using the Smart Tally protocol.

## Initiation

When using a smart device or computer with a full browser implementation, the initial connection sequence to the Roland switch via that devices' web browser will cause the Roland switch to send a web page containing a set of scripts used by the browser allowing the user to select a specific tally channel to monitor and then for subsequent status requests to the Roland switch.

The STAC, having no capacity to run the scripts sent by the Roland switch, bypasses that initial initialization sequence. It instead establishes a connection to the Roland switch and then immediately starts to poll the Roland switch for the tally status of the channel selected by the user during setup of the STAC. 

## Status Request

When operating, the STAC continuously polls the Roland switch by sending to it an ASCII character string in the following format:

 `GET /tally/[number]/status`  
 followed by a `<return>` and a `<newline>` character.
 
Where: `[number]` is the ASCII integer representation of the tally channel number of the Roland device for which status is being requested; `1` to `8`.
 
This string is sent to the IP address and port number specified by the user when the STAC is configured via a web browser. Refer to the _STAC Users Guide.md_ in this repository.

## Expected Response

The STAC is expecting a response to a status request to be an ASCII character string that is exactly one of: 
 
`onair`  
`selected`  
`unselected`  

With no `<return>` or `<newline>` characters appended.

Capitalization is significant.

Any other response will cause the STAC to display an error state as described in the *Troubleshooting* section of the *STAC Users Guide.md*.

## Other Considerations

The STAC sends no identifiable information directly in the status request. Thus in emulated environments, using the requested tally channel number as a STAC  identifier may limit the number of physical STACs that can be used.

Other methods, such as using the MAC address or IP address assigned to a STAC from the router, may be considered.

## Regarding the V-160HD

Implementation of the Smart Tally protocol on the V-160HD differs from the V-60HD. The V-160HD does not respond to a "short form" of a tally request, instead using HTML compliant standard protocol to query the switch.

The V-160HD also implements and requires Basic Authentication for all HTML interactions, where the user ID (at the time of writing) is fixed to be '`user`' and the password is a four-digit number selectable by the user, with a default of '`0000`'.

When configured for the V-160HD, the STAC bypasses the normal '`GET /`' request that a browser would do when first connecting to a URL. In place it immediately starts polling for status of the channel configured by the user in the format:

`GET /tally/{CHAN_BANK}{BANK_NUM}/status\r\n\r\n`

where:
`CHAN_BANK` is either `hdmi_` or `sdi_`

`BANK_NUM` is an ASCII integer from `1` to `8` representing the HDMI or SDI input of the V-160HD.

This is sent with the Basic Authentication and other headers as required.

The expected response in the return payload is as per the V-60HD described above.

As the interaction with the V-160HD is standard HTML, the referring IP address of the STAC making the tally request is included in the headers from the STAC to the switch so this information may be of use when using the STAC in emulated environments.
<br><br>

---
### Revision History
**2024-05-09:** Revise for support of the V-160HD.  
**2021-04-08:** Add "Other Considerations" section. Correct typos.  
**2021-04-05:** First release.

 
