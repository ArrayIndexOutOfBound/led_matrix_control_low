# led_matrix_control_low
Code source, bas niveau, en C, pour contrôler des matrices de LEDs
(Adafruit par exemple https://www.adafruit.com/product/2278)

Le code de ce repository se base sur le code suivant : https://github.com/hzeller/rpi-rgb-led-matrix.
- Un grand merci à `hzeller` pour avoir fourni un tel code !

Il faut de se fait, que la raspberry possède un HAT et qu'elle soit relié aux panneaux.
- https://www.adafruit.com/product/2345
- https://cdn-shop.adafruit.com/970x728/2345-12.jpg

Ce code fonctionne à été testé sur :
- Raspberry PI 3 B+
- Raspberry PI 4 B+

Le code principal se trouve dans `examples-api-use`.
- Mathieu-Panel.cc est la première version du code, affichant un `string` de X caractères avec un couleur défini
- Mathieu-Pixel.cc est une nouvelle version, pas prête, qui afficherai directement en pixel par pixel avec chacun sa couleur

L'objectif, est qu'avec la dérivation de `text-example.cc`, et une sorte "d'API" en C on puisse modifier l'affichage avec une interface web et un code en Python [se situant ici](https://github.com/ArrayIndexOutOfBound/led_matrix_control_high)

# Dépendances

! S'il y a un manque de dépandance !
