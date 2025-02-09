# Despertador Melódico

Este projeto implementa um despertador interativo que reproduz um alarme melódico até que o usuário toque a sequência correta de notas, incentivando a prática musical logo ao acordar.

## Requisitos

- [PicoSDK](https://github.com/raspberrypi/pico-sdk)  
  (É necessário baixar o PicoSDK. Um script chamado `pico-setup` está disponível para facilitar a instalação.)

## Compilação e Instalação

1. **Baixar o PicoSDK:**
   Clone o repositório do script `pico-setup` e execute-o para configurar o ambiente:
   ```bash
   git clone https://github.com/raspberrypi/pico-setup

   cd pico-setup
   ./setup.sh
   ```

2. **Compilar o Projeto:**
   No diretório raiz do projeto, execute:
   ```bash
   ./build_project
   ```
   Este comando irá compilar o código e gerar o arquivo binário (`.uf2`).

3. **Upload para a Placa:**
   - Mantenha pressionado o botão \textbf{Debug} na placa BitDogLab.
   - Conecte a placa ao computador via USB.
   - Copie o arquivo `.uf2` gerado para a placa (ela aparecerá como um dispositivo de armazenamento).

## Descrição

O \emph{Despertador Melódico} integra um alarme tradicional com um mecanismo interativo de desativação, onde o usuário precisa tocar uma sequência correta de notas (utilizando um algoritmo de detecção baseado em Goertzel) para desligar o alarme. Isso incentiva a prática musical logo ao acordar.
