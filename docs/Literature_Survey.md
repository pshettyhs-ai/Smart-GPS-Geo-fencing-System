# Literature Survey

A review of prior published work that informed the design direction of this
project.

---

### 1. Geofencing Application using IRNSS/NavIC
**Author:** Rupam Prasadh
**Published in:** 2022 2nd International Conference on Intelligent
Technologies (CONIT)

This paper surveys the broader space of location-based services (LBS) that
have become possible with the growth of satellite navigation systems and
smart devices, and focuses specifically on geofencing as a way to monitor
predefined boundaries remotely. As a case study, the authors build a child
safety system: a hardware unit transmits a child's live coordinates, and a
companion mobile app lets a parent define a geofence and receive an alert the
moment the child leaves it. Notably, the positioning in this work is based on
**India's NavIC** satellite system rather than GPS, which is relevant context
for projects deployed within India. This project borrows the core
entry/exit-alert concept but implements it as a standalone GSM/SMS hardware
device rather than an app-based one.

### 2. Design and Development of a Geofencing-Based Attendance System for Mobile Applications
**Author:** Rajeshwari Shinde
**Published in:** 2022 10th International Conference on Emerging Trends in
Engineering and Technology – Signal and Information Processing
(ICETET-SIP-22)

This paper proposes replacing biometric attendance hardware (fingerprint,
RFID, retina scanners) — which the authors note is costly and slow to scale
across a large workforce — with a geofenced mobile app. Employees are
automatically marked present or absent based on whether their phone is
located inside a geofence drawn around the workplace, removing the need for
dedicated scanning hardware entirely. This motivates one of the applications
listed for this project (`Applications.md`, "Workplace Attendance"), even
though this repository's implementation targets a dedicated hardware tracker
rather than a phone-based app.

### 3. Human Safety Using GPS and Geofencing
**Author:** K. S. Yamuna
**Published in:** 2024 4th International Conference on Sustainable Expert
Systems (ICSES)

This paper targets personal safety — particularly women's safety in both
urban and rural settings — by combining live GPS tracking with geofenced
"safe zones" (home, workplace, school). The proposed system automatically
notifies pre-registered emergency contacts and/or authorities the moment a
user leaves a designated safe zone, alongside manual emergency-alert
features. This is one of the closest precedents to the entry/exit SMS-alert
behaviour implemented in this repository's firmware, and reinforces the
choice of an SMS-based (rather than purely app-based) alert channel, since
it does not depend on the at-risk user's phone having mobile data or app
connectivity at the moment of an incident.

---

## How this project differs

All three papers above describe **app-connected or cloud-connected**
systems. This project's contribution is a **fully standalone embedded
device** — no smartphone app, no server, and no internet dependency on the
tracked side. It only requires 2G GSM signal to send an SMS, which makes it
suitable for low-connectivity or rural deployments where a data-dependent app
solution may not reliably function.
