import { createPublicClient, createWalletClient, http, parseAbi } from "viem"
import { arbitrumSepolia } from "viem/chains"
import { privateKeyToAccount } from "viem/accounts"
import { stringToBytes } from "viem/utils"; // Import utility to convert string to bytes
import "dotenv/config"

const ABI = parseAbi([
	"function register_developer() public returns (string)",
	"function submit_app_hash(address,bytes32) public returns (string)",
	"function verify_app_hash(bytes32) public returns (bytes32)",
])

const account = privateKeyToAccount((process as any).env.PRIVATE_KEY)

const client = createWalletClient({
  chain: arbitrumSepolia,
  transport: http(),
  account,
})

const publicClient = createPublicClient({
  chain: arbitrumSepolia,
  transport: http(),
})

// https://sepolia.arbiscan.io/address/const CONTRACT_ADDRESS = "0x46be8751225be83d7a9b97fec0214c53795d8477"
const CONTRACT_ADDRESS = "0xa46d9c1ee08938525462349e0296ffff05057cc7"

async function register() {
    //const rawName = stringToBytes("Name", { size: 32 }); // Convert string to raw bytes
    const result = await client.writeContract({
        abi: ABI,
        address: CONTRACT_ADDRESS,
        functionName: "register_developer",
        //args: [rawName],
    });

    console.debug(`Developer registered: ${result}`);
    return result;
}

async function submit() {
    const developerAddress = "0xaB55FD34340b5e8fEE8615690431649046fE135b";
    const appHash = "0xabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdeaaebd";

    const nonce = await publicClient.getTransactionCount({ address: developerAddress });
    const result = await client.writeContract({
        abi: ABI,
        address: CONTRACT_ADDRESS,
        functionName: "submit_app_hash",
        args: [developerAddress, appHash],
	nonce: nonce + 1,
    });

    console.debug(`App hash submitted: ${result}`);
    return appHash;
    //return appHash; // Return the appHash so it can be used by verify()
}

async function verify(newAppHash) {
    //const appHash = "0xabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdeaaebd";
    //const developerAddress = "0xaB55FD34340b5e8fEE8615690431649046fE135b";

    const result = await publicClient.readContract({
        abi: ABI,
        address: CONTRACT_ADDRESS,
        functionName: "verify_app_hash",
        args: [newAppHash],
    });

    console.debug(`Val of newapphash: ${newAppHash}`);
    //console.debug(`Val of dev: ${registeredDev}`);
    console.debug(`Verification result: ${result}`);
}

async function testContract() {
    const registeredDev = await register(); // Register the developer
    const newAppHash = await submit(); // Submit the app hash
    await verify(newAppHash); // Verify the app hash
}

testContract().catch(console.error);


//register()
//submit()
//verify()

/*async function write() {
  const result = await client.writeContract({
    abi: ABI,
    address: CONTRACT_ADDRESS,
    functionName: "register_developer",
    //functionName: "set_value",
    args: ["Devname"],
  })

  console.debug(`Contract: ${result}`)
}

async function read() {
  const result = await publicClient.readContract({
    abi: ABI,
    address: CONTRACT_ADDRESS,
    functionName: "verify_app_hash",
    args: ["0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"],
    //functionName: "submit_app_hash",
    //functionName: "register_developer",
    //functionName: "dumbtest",
  })

  console.debug(`Contract: ${result}`)
}*/

//read()
//write()
