# Viseci vrtovi
Projekat iz predmeta Racunarska grafika na Matematickom faultetu. 

Skolska godina 2022/2023.

Za izradu je korišćen skelet projekta: `https://github.com/matf-racunarska-grafika/project_base.git` 
i materijali sa [LearnOpenGL](https://learnopengl.com/).

[Link](https://www.youtube.com/watch?v=qexc3Id7qw8) ka video demonstraciji projekta.

---------------------------------------

# Opis
Projekat je implementacija staticne scene. Srediste scene predstavlja model grada. Oko njega su prikazani instancirani modeli drveta. 
Osim ovih elemenata na sceni se nalaze i kameni mostovi i platforme.

Na sceni su osim, direkcionog svetla implementirana i dva pokretna izvora svetlosti.
Jedan izvor emituje svetlost plave, a drugi crvene boje. Njihovim kombinovanjem nastaje bogat spekatr nijansi osvetljenja koji se najbolje uocava na kamenim elementima scene.

---------------------------------------

# Uputstva:

`git@github.com:ankastankovic98/grafika_april_2023.git`

- Pomeranje kamere `W` `S` `A` `D`.

- Pritiskom na dugme `H` aktivira se HDR efekat.

- Pritiskom na dugme `B` aktivira se Bloom efekat.

- Parematar gama i ekspoziciju za HDR efekat moguce je dodatno podesavatai pomocu GUI interfejsa koji se aktivira pritiskom na `F1`.

- Moguce je podesavatai izvor direkcionog svetla na sceni, kao i podesavanja kamere pomocu GUIa.

- Osim HDR efekta moguce je izabrati i dodatne efekte koji ce se primeniti na sceni:
  - Blur
  - Ruzicasti svet
  - Iscrtavanje linijama

- Pritiskom na `ESC` zatvara se aktivni prozor.

---------------------------------------

# Sadrzaj:

U projektu su implementirane sve obavezne oblasti (Blending, Face culling, Advanced lighting).

Iz grupe A implementirane su oblasti:

- Framebuffers (blur) 
- Cubemaps( skybox)
- Instancing

Iz grupe B implementirane su oblasti HDR i Bloom.

---------------------------------------

# Korisceni resrsi:

[Grad](https://sketchfab.com/3d-models/lowpoly-cartoon-shanghai-18da454727524cfe9c2b61bc5f7e3c34)

[Bonsai drvo](https://www.turbosquid.com/3d-models/3d-model-hand-painted-tree-1150298)

[Most](https://www.turbosquid.com/3d-models/3d-stone-bridge-base-model-1929747)

[Kamena platforma](https://www.turbosquid.com/3d-models/3d-stone-platforms-moss-1-1975070)

[Skybox tekstura](https://www.freepik.com/free-vector/mars-landscape-alien-planet-desert-background_28877272.htm#page=2&query=space%20cartoon&position=31&from_view=search&track=ais)
    
    



