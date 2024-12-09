#    SECURLY

## Requisitos, guía de preparación e info del SDK de Stylus abajo.

## Disclaimer

  Como reto decidí hacer este proyecto de manera individual con un mínimo de ayuda (más ChatGPT), en un Hackathon de 48h (más otras 24h de workshops).
  No tenía conocimientos previos de programar contratos inteligentes, ni de Arbitrum, ni del SDK de Stylus.
  Por estas razones, el backend no funciona como querría exactamente, tampoco hay frontend integrado, y el proyecto final está lejos de lo que imaginaba.
  Por lo que describiré la visión que tenía en mente del proyecto, y luego lo que he conseguido realizar en este tiempo.

# CÓMO SERÍA UNA VERSIÓN COMPLETAMENTE FUNCIONAL DE SECURLY?

## Para desarrolladores

  Gracias a Securly, los desarrolladores pueden tener una ID única, y desarrollar aplicaciones bajo esa ID.
  La Blockchain verificaría que esa persona ha desarrollado dichas aplicaciones y mostraría una lista de ellas.
  Por lo que los usuarios podrían ver que las aplicaciones están creadas por un desarrollador de confianza, y no tengan miedo de ser estafados.
  Esto permitiría que los desarrolladores puedan centrarse más en desarrollar, y no se preocupen tanto de prevenir que sus usuarios sean estafados, ya que añade una barrera extra de seguridad.
  Los usuarios, al sentirse más seguros, usarían más las aplicaciones.
  
  En un futuro, los desarrolladores podrían interactuar con los usuarios de una manera más directa desde Securly.
  Como ejemplo, los usuarios podrían donar a sus proyectos favoritos desde Securly, evitando cualquier middleman y asegurándose de que sus donativos llegen al desarrollador.

## Para usuarios

  Los usuarios podrían verificar quién ha desarrollado cada aplicación.
  Así asegurándose de que sus probabilidades de caer en estafas, principalmente el phising, disminuyan significativamente.
  Esto aumentaría la seguridad del usuario promedio, ya que una gran parte de las estafas están causadas por pequeños despistes.
  Por lo que nosotros permitimos que sea fácil y rápido verificar quién ha hecho qué.

## Visión futuro

  Securly podría ir más allá de solo verificar aplicaciones y desarrolladores, podría usarse para verificar vendedores online, NFTs, influencers, etc...

# CÓMO ES LA VERSIÓN DE SECURLY QUE HE CONSEGUIDO IMPLEMENTAR

## Almacena direcciones de desarrolladores

  El contrato obtiene la dirección del desarrollador y la registra en la Blockchain:

  ```
    ArbResult register_developer(uint8_t *input, size_t len) {

  uint8_t *developer_address = get_sender(); // Get the caller's address
  uint8_t slot[32];
  storage_combine_keys((uint8_t *)STORAGE_SLOT__developers, developer_address, slot);

  // Check if already registered (doesn't work, can re-register addresses)
  storage_load_bytes32(slot, buf_out);
  if (!bebi32_is_zero(buf_out)) {
    return _return_short_string(Failure, "AlreadyRegistered");
  }

  return _return_short_string(Success, "Registered");
}
```

Actualmente no checkea correctamente si ya se ha subido una dirección, ya que deja registrarse varias veces con la misma clave.
Asumo que no estoy almacenando correct
También quería vincular un nombre a cada dirección, pero no fui capaz.

## Crea un hash nuevo obtenido del `app_hash`, y puede que vincule dicho hash al desarrollador

```
ArbResult submit_app_hash(uint8_t *input, size_t len) {
    if (len != 64) {		//Error triggers when re-registering app (instead of HashExists)
        return _return_short_string(Failure, "InvalidInputLength");
    }

    uint8_t developer_address[32];
    uint8_t app_hash[32];

    // Split input into developer address and app hash
    memcpy(developer_address, input, 32);
    memcpy(app_hash, input + 32, 32);

    // Check if the app hash already exists
    uint8_t app_slot[32];
    generate_app_key(app_hash, app_slot);
    storage_load_bytes32(app_slot, buf_out);

    if (!bebi32_is_zero(buf_out)) {
        return _return_short_string(Failure, "HashExists");
    }

    // Link the app hash to the developer address
    storage_cache_bytes32(app_slot, developer_address);
    storage_flush_cache(false);

    return _return_short_string(Success, "AppHashStored");
}
```

  La función es capaz de generar un nuevo hash, y se asegura de que la misma aplicación no pueda ser subida de nuevo (aunque no da el error que debería).
  También intenta vincular la aplicación con el desarrollador, pero creo que no lo hace correctamente.

# Intenta verificar que la aplicación está vinculada al desarrollador

  ```
  ArbResult verify_app_hash(uint8_t *input, size_t len) {
  if (len != 32) {
    return _return_short_string(Failure, "InvalidHashLength");
  }

  uint8_t app_hash[32];
  uint8_t slot[32];

  memcpy(app_hash, input + 32, 32);
  generate_app_key(app_hash, slot);

  // Load the developer address linked to the app hash
  storage_load_bytes32(slot, buf_out);

  // Return the developer address (Doesn't work properly return other address)
  return _return_success_bebi32(slot);
}
```

  La función debería retornar la dirección del desarrollador si detecta que están vinculados, pero actualmente solo retorna una dirección basura que tiene nada que ver con la del desarrollador.


# INFO DEL SDK DE STYLUS

## Register and Storage Slots

Storage slots are pretty similar to registers in 32bit architectures.
Since both have:

1. Fixed Size: Storage slots in Ethereum can hold exactly 32 bytes. Similar to how registers have a fixed size in assembly (e.g., 32-bit, 64-bit).

2. Direct Addressing: Just as you access a specific register by its identifier (eax, r1), you access storage slots using their indexes: 0x01, 0x02.

3. Explicit Management: Like assembly programming, developers must explicitly manage how storage slots are allocated and used. Miss assignment of memory/slots can cause data corruption.

## Requisitos

- Brew (Mac) / Chocolatey (Windows)
- Docker (Docker Desktop)
- rust, cargo y rustup
- LLVM (con wasm-ld): Disponible desde la versión 15 (`llvm@15`)

## Verificar si tenemos `wasm-strip` instalado

Si no, podemos instalarlo con `brew install wabt`

## Instalando Cargo Stylus

- fork / git clone de este repositorio
- `git submodule update --init --recursive`
- `cargo install cargo-stylus`
- `rustup target add wasm32-unknown-unknown` (**opcional**, solventa un error de target wasm32 not found al hacer make)
- `npm install`

## Validar entorno de desarollo

- `make` para generar el archivo contract.wasm 
- `cargo stylus check --wasm-file ./contract.wasm -e https://sepolia-rollup.arbitrum.io/rpc` (si tenemos output en verde estamos listos 🚀🚀)

## Una vez esté validado, añadir archivo clave privada

- `touch .env` crea archivo para clave privada
- Dentro escribir: `PRIVATE_KEY=0xtu_clave_privada`

## Para probar

- Ejecutar `make play`
- Modificar valores (principalmente `developerAddress` y `appHash`) para probar con otras claves.
- Ejecutar múltiples veces `make play` con la misma appHash da error (como debería), developerAddress no da ese problema.
