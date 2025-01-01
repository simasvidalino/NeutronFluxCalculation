#!/bin/bash

readonly BINARY="../build/bin/CalFluxNeutron"
readonly DEST_DIR="/home/andreia-simas/Desktop/NeutronFluxCalculation/build/DEPENDENCIES/"

for lib in $(ldd "$BINARY" | grep -oP '(?<= => ).*(?= \()'); do
    # Copia o arquivo real (a versão completa) primeiro
    if [ -L "$lib" ]; then
        real_lib=$(readlink -f "$lib")
        cp "$real_lib" "$DEST_DIR"
        
        # Cria novamente o link simbólico no diretório de destino
        ln -sf "$(basename "$real_lib")" "$DEST_DIR/$(basename "$lib")"
    else
        # Copia bibliotecas normais (não links simbólicos)
        cp "$lib" "$DEST_DIR"
    fi
done

