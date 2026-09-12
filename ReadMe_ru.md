# Pseudo3d Engine

C++, OpenGL 3.3, [stb_image](https://github.com/nothings/stb), [GLEW](https://github.com/nigels-com/glew)

Для тестов: [SFML](https://github.com/SFML/SFML)

# О проекте

Вас никогда не удивляло, на сколько красивым может быть псевдо-3d? И ведь это не просто "эстетика", оно требует меньше вычислительных ресурсов! А если применить хитрость, то может быть не столь очевидно, что вся карта в 2d...
Моя библиотека `pseudo3d_engine`, позволяет задавать 2d карту из стен и отрисовывать её в псевдо-3d. Библиотека пока находиться в разработке, но уже есть: зеркала, поддержка прозрачности у текстур стен, порталы, которые могут вести на другую карту, текстурированные пол и потолок (или небо), и ещё по-мелочи

![image](data/readme_image_0.png)

![image](data/readme_image_1.png)

![image](data/readme_image_2.png)

<!-- link to wiki:example -->

# О совместимости

Библиотека основывается на OpenGL, так что потенциально она может работать на любой ОС с поддержкой графики. Но протестировано, пока лишь на Linux

# Как установить в систему?

Для это потребуется `cmake`, `make`, `gcc`

Сперва нужно скачать сам проект: либо по [ссылки](https://github.com/3NikNikNik3/pseudo3d_engine/archive/refs/heads/main.zip), либо через git

	git clone https://github.com/3NikNikNik3/pseudo3d_engine.git

После запускаем:

	cmake -Bbuild

По умолчанию, устанавливается в `/usr/local`, если нужно в другое место, то добавьте `-DCMAKE_INSTALL_PREFIX=dir`. Например:

	cmake -Bbuild -DCMAKE_INSTALL_PREFIX=/usr

И финальное:

	cd build
	make install

Всё! Можно удалять папку `pseudo3d_engine/`

<!-- link to wiki:build -->
