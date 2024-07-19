import {
  Text,
  View,
  TouchableOpacity,
  StyleSheet,
  TextInput,
  ToastAndroid,
} from "react-native";
import React, { useState, createContext, useContext, useEffect } from "react";
import { createNativeStackNavigator } from "@react-navigation/native-stack";
import Slider from "@react-native-community/slider";
import {
  getAuth,
  createUserWithEmailAndPassword,
  signInWithEmailAndPassword,
} from "firebase/auth";
import { initializeApp } from "firebase/app";
import { getFirestore } from "firebase/firestore";
import { doc, setDoc, getDoc } from "firebase/firestore";

const firebaseConfig = {
  apiKey: "AIzaSyAyut07XIb2UhfdMEim0V1hMRAW7B43umc",
  authDomain: "urban-voice-5bab7.firebaseapp.com",
  projectId: "urban-voice-5bab7",
  storageBucket: "urban-voice-5bab7.appspot.com",
  messagingSenderId: "561079818730",
  appId: "1:561079818730:web:788d296694bc78b9a53a93",
};

const app = initializeApp(firebaseConfig);
const auth = getAuth(app);
const Stack = createNativeStackNavigator();
const db = getFirestore(app);

let Usuario = createContext("");
let Valor = createContext(15);
let TempoVotar = createContext(0);
let JaVotou = createContext(false);

export default function Index() {
  return (
    <Stack.Navigator initialRouteName="Home">
      <Stack.Screen name="Home" component={Home} />

      <Stack.Screen
        options={{ title: "Pagina de Login" }}
        name="LoginPage"
        component={LoginPage}
      />

      <Stack.Screen
        options={{ title: "Pagina de Votação" }}
        name="VotingPage"
        component={PaginaVotar}
      />
    </Stack.Navigator>
  );
}

const Home = ({ navigation }) => {
  return (
    <View style={styles.container}>
      <Titulo></Titulo>

      <TouchableOpacity
        style={styles.button}
        onPress={() => navigation.navigate("LoginPage")}
      >
        <Text style={styles.buttonText}>Entrar em sua conta</Text>
      </TouchableOpacity>
    </View>
  );
};

export const SliderTemperatura = (props) => {
  let valor = useContext(Valor);

  return (
    <View style={{ alignItems: "center" }}>
      <TextoTemperaturaAtual
        style={{ fontSize: 30, marginTop: "30%" }}
      ></TextoTemperaturaAtual>
      <Text style={{ fontSize: 30, marginBottom: "20%" }}>{valor}°C</Text>
      <Slider
        style={{
          width: 175,
          height: 40,
          transform: [{ scaleY: 1.5 }, { scaleX: 1.5 }],
        }}
        minimumValue={15}
        maximumValue={30}
        step={1}
        minimumTrackTintColor="#FFFFFF"
        maximumTrackTintColor="#000000"
        thumbTintColor="red"
        {...props}
        value={valor}
        onValueChange={(...args) => {
          props.setter(...args);
          console.log(valor);
        }}
      />
    </View>
  );
};

const PaginaVotar = ({ }) => {
  let [valor, setValor] = useState(15);
  let [jaVotou, setJaVotou] = useState(false);
  let usuario = useContext(Usuario);

  let [ri, gi, bi] = [134, 206, 235]; // cor da temperatura fria (azul)
  let [r, g, b] = [204, 92, 92]; // cor da temperatura quente (avermelhado)

  const vote = async () => {
    setJaVotou(true);
    console.log(`votando: ${valor}`);
    try {
      console.log("eba"); // temp
      let votos = (await getDoc(doc(db, "Votos1", "Votos")))["data"]();
      console.log(votos);
      for (let i = 0; i <= 30; i++) {
        if (votos[`${i}`] != "0") {
          continue;
        }

        let Obj = {};
        Obj[`${i}`] = `${valor}`;
        await setDoc(doc(db, "Votos1", "Votos"), Obj, {
          merge: true,
        });

        ToastAndroid.show("Voto registrado!", ToastAndroid.SHORT);
        break;
      }
    } catch (e) {
      console.error("Error adding document: ", e);
      setJaVotou(false); // como deu erro, nao devemos bloquear o voto
    }
  };

  return (
    <JaVotou.Provider value={jaVotou}>
      <Valor.Provider value={valor}>
        <View
          style={{
            height: "100%",
            backgroundColor: `rgb(${ri + ((r - ri) / (30 - 15)) * (valor - 15)},${gi + ((g - gi) / (30 - 15)) * (valor - 15)},${bi + ((b - bi) / (30 - 15)) * (valor - 15)})`,
            display: "flex",
            flexDirection: "column",
            alignItems: "center",
          }}
        >
          <Titulo></Titulo>
          <SliderTemperatura setter={setValor}></SliderTemperatura>
          <View
            style={{
              marginTop: "10%",
              marginBottom: 110,
              alignItems: "center",
            }}
          >
            <TimerVotar
              style={{ opacity: Number(jaVotou) }}
              setter={{ setJaVotou }}
            ></TimerVotar>
            <TouchableOpacity
              style={jaVotou ? styles.disabledButton : styles.voteButton}
              disabled={jaVotou}
              onPress={vote}
            >
              <Text style={styles.buttonText}>Votar !</Text>
            </TouchableOpacity>
          </View>
        </View>
      </Valor.Provider>
    </JaVotou.Provider>
  );
};

const TextoTemperaturaAtual = (props) => {
  let [temperatura, setter] = useState(0);
  const atualizar = async () => {
    let log = await getDoc(doc(db, "Votos1", "Logs"));
    console.log(log["data"]());
    setter(Number(log["data"]()["log"]));
  };
  useEffect(() => {
    atualizar();
    this.interval = setInterval(() => {
      try {
        atualizar();
      } catch { }
    }, 10000);
    return () => {
      clearInterval(this.interval);
    };
  });
  return (
    <View>
      <Text>Temperatura atual: {temperatura}°C</Text>
    </View>
  );
};

const TimerVotar = (props) => {
  // temp
  /* no futuro:
  ultimoTempo = pegarTempoDaUltimaVotacao();
  tempoPassado = tempoAtual - ultimoTempo;
  setar timer como () => {
    if tempoPassado >= 300 {
      desvotar(); (pegar o setter em uma prop)
    }
  }
  */
  let jaVotou = useContext(JaVotou);
  let [timer, setTimer] = useState(10);
  useEffect(() => {
    this.interval = setInterval(() => {
      if (!jaVotou) {
        return;
      }
      setTimer(timer - 1);
      if (timer <= 0) {
        //desvotar
        console.log("desvotarei");
        props.setter.setJaVotou(false);
        setTimer(10);
      }
    }, 1000);
    return () => {
      clearInterval(this.interval);
    };
  }, [timer, jaVotou]);

  return (
    <TempoVotar.Provider value={timer}>
      <View>
        <Text>{timer}s</Text>
      </View>
    </TempoVotar.Provider>
  );
};

const LoginPage = ({ navigation }) => {
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");

  const handleCadastro = ({ }) => {
    createUserWithEmailAndPassword(auth, email, password).then(
      (_userCredential) => {
        navigation.navigate("VotingPage");
      },
    );
  };

  const handleLogin = ({ }) => {
    try {
      signInWithEmailAndPassword(auth, email, password).then(() => {
        navigation.navigate("VotingPage");
      });
    } catch { }
  };

  return (
    <Usuario.Provider value={email}>
      <View style={styles.container2}>
        <Text
          style={{
            fontSize: 50,
            color: "midnightblue",
            fontWeight: "bold",
            marginTop: 0,
          }}
        >
          Urban Voice
        </Text>

        <TextInput
          style={styles.input}
          placeholder="Email"
          autoCapitalize="none"
          value={email}
          onChangeText={setEmail}
        />
        <TextInput
          style={styles.input}
          placeholder="Senha"
          secureTextEntry
          value={password}
          onChangeText={setPassword}
        />
        <TouchableOpacity
          onPress={handleLogin}
          style={{
            width: 100,
            backgroundColor: "mintcream",
            justifyContent: "center",
            alignContent: "center",
            borderRadius: 10,
            marginTop: 0,
            marginBottom: 10,
          }}
        >
          <Text style={styles.buttonText}>Login</Text>
        </TouchableOpacity>

        <TouchableOpacity
          onPress={handleCadastro}
          style={{
            width: 100,
            backgroundColor: "mintcream",
            justifyContent: "center",
            alignContent: "center",
            borderRadius: 10,
            marginTop: 0,
            marginBottom: 0,
          }}
        >
          <Text style={styles.buttonText}>Cadastro</Text>
        </TouchableOpacity>
      </View>
    </Usuario.Provider>
  );
};

const Titulo = () => {
  return (
    <View
      style={{
        marginTop: 110,
        width: "100%",
        justifyContent: "center",
        alignItems: "center",
        //textAlign: 'center',
      }}
    >
      <Text
        style={{
          fontSize: 50,
          color: "midnightblue",
          fontWeight: "bold",
        }}
      >
        Urban Voice
      </Text>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    height: "100%",
    backgroundColor: "skyblue",
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
  },
  button: {
    width: "50%",
    backgroundColor: "mintcream",
    justifyContent: "center",
    alignContent: "center",
    borderRadius: 20,
    marginTop: "90%",
    marginBottom: 110,
  },
  voteButton: {
    width: "25%",
    backgroundColor: "mintcream",
    justifyContent: "center",
    alignContent: "center",
    borderRadius: 20,
  },
  disabledButton: {
    width: "25%",
    opacity: 0.5,
    backgroundColor: "mintcream",
    justifyContent: "center",
    alignContent: "center",
    borderRadius: 20,
  },
  buttonText: {
    color: "midnightblue",
    fontSize: 18,
    margin: 10,
    textAlign: "center",
    fontWeight: "bold",
  },
  container2: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
    backgroundColor: "skyblue",
  },
  input: {
    width: "70%",
    height: 40,
    borderColor: "white",
    backgroundColor: "white",
    borderWidth: 1,
    marginBottom: 20,
    paddingHorizontal: 10,
    borderRadius: 10,
  },
  error: {
    color: "red",
    marginTop: 10,
  },
});
