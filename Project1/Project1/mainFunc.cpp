#include "m.h"
#include <graphics.h>
using namespace std;

float smoothingRadius = 12;                //���÷�Χ(ѹ����ճ����)
float coefficient = -20;                   //ѹ������ϵ��
float targetDensity = 0.1;                 //Ŀ���ܶ�
float board = 20;                          //ǽ�ڳ���ϵ��
float collision_damp = 0.1;                //ǽ�ڷ�������
float push_r = 15;                         //��������Χ
float push_f = 300;                        //������ϵ��
float viscosity = 0.6;                     //ճ��ϵ��




//��Ҫ�޸ĵ�
int ids = 0;                               //��id������
int num = 0;                               //�������������
int num2 = 0;



class Ball {
public:
	vector<float> velocity;
	vector<float> accelerated;
	vector<float> position;
	vector<float> real_pos;
	float mass;
	float size;
	//float act_range;
	float q;
	int id;


	Ball(int id,vector<float> p, float size = 1, float mass = 1, vector<float> v = { 0,0 }) {
		velocity = v;
		accelerated = { 0,0 };
		this->mass = mass;
		this->size = size;
		position = p;
		real_pos = p;
		this->id = id;
	}

	//��ϣ��ȽϺ�������
	bool operator==(const Ball& p)const {
		return (id == p.id);
	}
};

template<>

//��ϣ����
class hash<Ball> {
public:
	size_t operator()(const Ball& p)const {
		return hash<int>()(p.id);
	}
};



class  Playground {
public:
	unordered_map<Ball, unordered_map<Ball, float>> dst_map;//�ϰ��������ϣ��
	vector<int> central_position;//������������(������Ϊ��������)
	vector<float> size;//������С
	vector<float> gravity;//����
	float time_interval;//ʱ������û���ϣ�
	float ratio;//���ű���
	
	COLORREF background;
	

	
	vector<Ball> items;

	Playground(float ratio = 1, vector<float> size = { 100,100 }, vector<int> cen = { 640, 480 }) {
		time_interval = 50;
		this->size = size;
		central_position = cen;
		this->ratio = ratio;
		gravity = { 0,0};
	}

	//�����򣬱�����Ϊ����Ϲ�ϣ������ģ�������ֲ�ѯ�ٶȲ�����㹴�ɶ���
	void addBall(vector<float> p, float size = 1, float mass = 1, vector<float> v = {0,0}) {
		Ball new_item(ids++,p, size, mass, v);
		
		//dst_map.insert(pair<Ball, unordered_map<Ball, float>>(new_item, {}));
		 
		//for (Ball item : items) {
		//	//dst_map[new_item][item] = 0;
		//	//dst_map[item][new_item] = 0;
		//	//reference_wrapper<float> re = ref(dst_map[item][new_item]);
		//	//re = ref(dst_map[new_item][item]);
		//	//dst_map[new_item].insert(pair<Ball, float>(item, 0));
		//	//dst_map[item].insert(pair<Ball, float&>(new_item, dst_map[new_item][item]));
		//}

		//dst_map[new_item][new_item] = 0;
		//dst_map[new_item].insert(pair<Ball, float>(new_item, 0));
		items.push_back(new_item);
		
	}
	

	//�ϰ������м������ಢ�����ڹ�ϣ����
	void calculateDsts() {
		float dst;
		for (int i = 0; i < items.size(); i++) {
			for (int j = i; j < items.size(); j++) {
				dst = calculateDst(items[i], items[j]);
				dst_map[items[i]][items[j]] = dst;
				//dst_map[items[j]][items[i]] = dst;
				num++;
				//printf("i:%d,j:%d----n:%d\n",i,j,num);
			}
		}
	}


	//�ϰ����ڹ�ϣ����������
	float findDst(Ball const& item1, Ball const& item2) {
		if (dst_map[item1].find(item2) != dst_map[item1].end()) {
			return dst_map[item1][item2];
		}
		else if (dst_map[item2].find(item1) != dst_map[item2].end()) {
			return dst_map[item2][item1];
		}
		return 100;
		
	}

	//��������ת��������
	vector<float> transform_position(vector<float> position) {
		return{central_position[0]+ratio*(position[0] -size[0]/2),central_position[1] + (size[1] / 2 - position[1]) * ratio};
	}


	//��������ת��������
	vector<float> retransform_pst(vector<float> p) {
		float x = (p[0] - central_position[0]) / ratio + size[0] / 2;
		float y = (central_position[1] - p[1]) / ratio + size[1] / 2;
		return{ x,y };
	}

	//ճ��������
	vector<float> calculateViscosityForce(Ball item) {
		vector<float> force = {0,0};
		float dst;
		//vector<float> res;
		for(Ball tItem : items){
			//dst = findDst(item, tItem);
			dst = calculateDst(item, tItem);
			num2++;
			if (dst > smoothingRadius) continue;
			for (int ca = 0; ca < 2; ca++) {
				force[ca] += (tItem.velocity[ca]-item.velocity[ca])*viscosity*infFunc(dst,smoothingRadius);
			}
		}
		return force;
	}

	//��������
	float infFunc(float dst, float radius) {
		if (dst >= radius) return 0;
		float volume = (M_PI * pow(radius, 4)) / 6;
		return(radius - dst) * (radius - dst) / volume;
	}

	//������������
	float infFuncDer(float dst, float radius) {
		if (dst >= radius)return 0;
		float scale = 12 / (pow(radius, 4) * M_PI);
		return (dst - radius) * scale;
	}

	//������㺯��
	float calculateDst(Ball const& item1, Ball const& item2) {
		float x,y;
		x = item1.position[0] - item2.position[0];
		y = item1.position[1] - item2.position[1];
		return sqrt(x*x+y*y);
	}

	//������㺯��
	float calculateDst(vector<float> item1,vector<float> item2) {
		float x, y;
		x = item1[0] - item2[0];
		y = item1[1] - item2[1];
		return sqrt(x * x + y * y);
	}

	//�ܶȼ��㺯��
	float calculateDensity(Ball const& item) {
		float density = 0;
		for (Ball const &tItem : items) {
			//float dst = findDst(item, tItem);
			float dst = calculateDst(item,tItem);
			if (dst > smoothingRadius)continue;
			float influence = infFunc(dst,smoothingRadius);
			density += item.mass * influence;
		}
		return density;
	}


	//ѹ��ϵ��
	float presCoefficient(float density) {
		return  (density - targetDensity)* coefficient;
	}

	float calculateSharedPressure(float A, float B) {
		float pA = presCoefficient(A);
		float pB = presCoefficient(B);
		return (pA + pB) / 2;
	}

	vector<float> resolut(vector<float> result) {
		vector<float> re = { 0,0 };
		float l = calculateDst(result, {0,0});
		re[0] = result[0] / l;
		re[1] = result[1] / l;
		return re;
	}

	//����ѹ��
	vector<float> calculatePressure(Ball const& item) {
		vector<float> pressure = { 0,0 };
		vector<float> res;
		for (Ball const& tItem : items) {

			float dst = calculateDst(item, tItem);
			//float dst = findDst(item, tItem);
			num2++;

			if (dst <= 0)continue;
			if (dst >= smoothingRadius)continue;

			float density = tItem.q;
			float slope = infFuncDer(dst, smoothingRadius);

			res = resolut({ item.position[0] - tItem.position[0],item.position[1] - tItem.position[1] });

			pressure[0] += calculateSharedPressure(density, item.q) * res[0] * slope * item.mass / density;
			pressure[1] += calculateSharedPressure(density, item.q) * res[1] * slope * item.mass / density;
		}
		return pressure;
	}


	//����������
	void setPressure() {
		vector<float> desitys;
		//calculateDsts();
		for (Ball& item : items) {
			float x = calculateDensity(item);
			desitys.push_back(x);
			item.q = x;
		}

		printf("%f\n", desitys[0]);

		//printf("\n");
		vector<float> pressure = { 0,0 };
		vector<float> viscosityForce = {0,0};
		//vector

		
		for (Ball& item : items) {
			//ѹ�����㲢��ֵ
			pressure = calculatePressure(item);
			for (int ca = 0; ca < 2; ca++) {
				if (item.q > 0) {
					item.accelerated[ca] += pressure[ca] / item.q;
					//item.velocity[ca] += pressure[ca] / desitys[i];
				}
			}
			viscosityForce = calculateViscosityForce(item);
			item.accelerated[0] += viscosityForce[0];
			item.accelerated[1] += viscosityForce[1];

		}

	}

	//��꽻����
	void pushBall(vector<float> p, bool push) {
		//vector<float> pressure = {0,0};
		for (Ball& item : items) {
			
			float dst = calculateDst(p, item.position);

			if (dst < push_r) {
				float x_weight = (item.position[0] - p[0]) / dst;
				float y_weight = (item.position[1] - p[1]) / dst;
				float slope = infFuncDer(dst, smoothingRadius);
				item.accelerated[0] += (push ? -1 : 1) * push_f*x_weight * slope * item.mass;
				item.accelerated[1] += (push ? -1 : 1) * push_f*y_weight * slope * item.mass;
				vector<float> a = transform_position(item.position);
				vector<float> b = transform_position({item.position[0]+ (push ? 1 : -1) * x_weight*slope*-1000*ratio,item.position[1]+ (push ? 1 : -1) * y_weight*slope*-1000*ratio});
				line(a[0], a[1], b[0], b[1]);
			}
		}
	}

	void ball_move() {
		//Ԥ���������һ��λ�ã�����Ϊ�����������
		for (Ball& item : items) {
			for (int ca = 0; ca <= 1; ca++) {//x��y��λ�ü���
				item.position[ca] += item.velocity[ca];
			}
		}

		//��������
		setPressure();


		for(Ball& item:items){
			//�˶�����
			for (int ca = 0; ca <= 1; ca++) {
				//����
				item.accelerated[ca] += gravity[ca];

				//��ֹ����
				if (item.position[ca] < size[ca] / 2) {
					item.accelerated[ca] += board * infFunc(size[ca] / 2 - abs((size[ca] / 2 - item.position[ca])), 5);
					//printf("accelerated%d = %f\n", ca, -board * infFunc(size[ca]/2 - abs((size[ca] / 2 - item.position[ca])), 5));
				}
				else {
					item.accelerated[ca] -= board * infFunc(size[ca] / 2 - abs((item.position[ca] - size[ca] / 2)), 5);
					//printf("accelerated%d = %f\n", ca, board * infFunc(size[ca]/2 - abs((item.position[ca] - size[ca] / 2)), 5));
				}

				//���ٶ�->�ٶ�
				item.velocity[ca] += item.accelerated[ca];
				item.accelerated[ca] = 0;
			}
			//printf("%f,%f\n", item.velocity[0], item.velocity[1]);
		}


		for (Ball& item : items) {
			for (int ca = 0; ca <= 1; ca++) {//x��y��λ�ü���
				//ǽ����ײ
				if (item.real_pos[ca] <= 0 || item.real_pos[ca] >= size[ca]) {
					if (item.real_pos[ca] <= 0)item.real_pos[ca] = 0;
					else if (item.real_pos[ca] >= size[ca])item.real_pos[ca] = size[ca];
					item.velocity[ca] = -item.velocity[ca] * (1 - collision_damp);//�ٶȷ�������
				}


				//�����ԡ�Ħ�������ĳ��ԣ�����Ч������
				//if (item.accelerated[ca] > 0) {
				//	item.accelerated[ca] -= 0.04;
				//}
				//else if (item.accelerated[ca] < 0) {
				//	item.accelerated[ca] += 0.04;
				//}

				//���׵�Ħ��������ʵȥ��Ҳû��ϵ
				if (item.velocity[ca] > 0) {
					item.velocity[ca] -= 0.0001;
				}
				else if (item.velocity[ca] < 0) {
					item.velocity[ca] += 0.0001;
				}
				item.real_pos[ca] += item.velocity[ca];
				item.position[ca] = item.real_pos[ca];

			}
		}

	}


	//�������
	void draw_playground() {
		setbkcolor(RGB(0,0,0));
		cleardevice();
		setfillcolor(background);
		setlinecolor(RGB(255,255,255));
		//setfillstyle(BS_HATCHED,HS_CROSS);
		setlinestyle(PS_SOLID, 2);
		vector<float> a, b;
		a = transform_position({0,size[1]});
		b = transform_position({size[0],0});
		fillrectangle(a[0], a[1], b[0], b[1]);
		//fillrectangle(central_position[0]-ratio*(size[0]/2), central_position[1] - ratio * (size[1] / 2), central_position[0] + ratio * (size[0] / 2), central_position[1] + ratio * (size[1] / 2));
	}


	//��ɫ����
	float colorFunc(float x) {
		float n = x / targetDensity;
		return max(0, min(255,255*(0.05*n*n-0.8*n+1.205)));
	}


	//�������
	void draw_items() {
		vector<float> p;
		//Ball item;
		setfillcolor(BLUE);
		for (Ball item :items) {
			setfillcolor(RGB(0,0,colorFunc(item.q)));
			p = transform_position(item.real_pos);
			solidcircle(p[0], p[1],item.size* ratio);
			//circle(p[0], p[1], smoothingRadius * ratio);
			//items[i].draw(ratio);
		}
	}

};





int main()
{
	// ��ʼ����ͼ����
	initgraph(640, 640);
	Playground pg(5, { 100,100 }, {329,320});
	pg.background = RGB(50,50,50);

	//������
	for (float x = 5; x <= 50; x += 2) {
		for (float y = 5; y <= 50; y += 2) {
			int vx = rand()%100, vy = rand() % 100;
			pg.addBall({x,y},1);
			printf("n:%d,items:%d\n",ids,pg.items.size());
		}
	}

	pg.draw_playground();
	//pg.draw_items();
	ExMessage msg;
	bool ld = false,rd = false;
	vector<float> pos = { 0,0 };
	bool g_down = 0;
	int g_open = 0;


	while (1) {

		BeginBatchDraw();//��ʼ����
		pg.ball_move();//���˶�
		pg.draw_playground();//��������
		pg.draw_items();//�����
		
		//�����¼�
		if (GetAsyncKeyState(VK_UP)) {
			g_down = 1;
		}
		else {
			if (g_down) {
				g_open += 1;
				g_open %= 5;
				g_down = 0;
				switch (g_open) {
				case(0):
					pg.gravity = {0,0};
					break;
				case(1):
					pg.gravity = { 0,-0.04 };
					break;
				case(2):
					pg.gravity = { -0.1,0 };
					break;
				case(3):
					pg.gravity = { 0,0.1 };
					break;
				case(4):
					pg.gravity = { 0.1,0 };
					break;
				}
			}
		}

		//����¼�
		if (peekmessage(&msg,EM_MOUSE)) {
			//msg = GetMouseMsg();
			pos = {float(msg.x),float(msg.y)};
			printf("����¼���������\n");
			switch (msg.message) {
			case WM_LBUTTONDOWN:
				rd = false;
				ld = true;
				break;
			case WM_LBUTTONUP:
				ld = false;
				break;
			case WM_RBUTTONDOWN:
				ld = false;
				rd = true;
				break;
			case WM_RBUTTONUP:
				rd = false;
				break;
			default:
				break;
			}
			flushmessage(-1);
		}

		//�������
		if (ld) {
			setlinecolor(RGB(255, 0, 0));
			circle(pos[0], pos[1], push_r * pg.ratio);
			pg.pushBall(pg.retransform_pst(pos),true);
		}

		//�Ҽ�����
		if (rd) {
			setlinecolor(RGB(255, 255, 255));
			circle(pos[0], pos[1], push_r * pg.ratio);
			pg.pushBall(pg.retransform_pst(pos), false);
		}
		printf("�����������:%d\n",num2);
		num2 = 0;
		FlushBatchDraw();//�������Ʋ���ʾ
		Sleep(10);
	}

	return 0;
}
