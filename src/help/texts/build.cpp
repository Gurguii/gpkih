namespace gpkih::help::build
{
	const char *usage = R"(== build ==
build certificates for a profile

[ syntax ]
  ./gpki build <profile> <entity_type> [subopts]
  
[ entity types ]
  ca : ca
  sv : server
  cl : client

[ subopts ]
  
  ** openssl **
  -algo    | --algorithm  <rsa,ed25519>      : algorithm to use
  -keysize | --keysize    <int>              : key size to use
  -outformat              <pem|der>          : output format
  -keyformat              <pem|der>          : key output format
  
  ** subject **
  -cn     | --common-name   <char[254]>   : set cn
  -serial | --serial        <char[254]>   : set serial
  -loc    | --location      <char[254]>   : set location
  -co     | --country       <char[254]>   : set country (2 letter code)
  -org    | --organisation  <char[254]>   : set organisation
  -st     | --state         <char[254]>   : set state
  -email  | --email         <char[254]>   : set email

  ** output **
  -inline | --inline : create inline file
  -pfx    | --pfx    : create pfx file (pkcs12), *will get prompted for password
)";
	const char *examples = R"(== build examples ==
      1. Building a CA
          ./gpkih build <myProfile> ca -cn <myCA>

      2. Building a SERVER avoiding user confirmation required
          ./gpkih build <myProfile> sv -cn <mySV> -country "ES" -location "Mi Casa" -y
      
      3. Build a CLIENT and create a .pfx file (will get prompted for password)
          ./gpkih build <myProfile> cl -cn <myCL> -pfx
      
      4. Build a CLIENT with all available options
          ./gpkih build <myProfile> cl -cn gurgui -co "ES" -lo "Gran Canarias" -org "Mariwanos" -mail "gurgui@mariwanos.com" -serial 200 -st "StateOfElevation" -pfx -algo rsa -keysize 2048 
    )";
}